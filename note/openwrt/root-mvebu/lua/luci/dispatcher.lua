--[[
LuCI - Dispatcher

Description:
The request dispatcher and module dispatcher generators

FileId:
$Id$

License:
Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

]]--

--- LuCI web dispatcher.
local fs = require "nixio.fs"
local sys = require "luci.sys"
local init = require "luci.init"
local util = require "luci.util"
local http = require "luci.http"
local nixio = require "nixio", require "nixio.util"

module("luci.dispatcher", package.seeall)
--[[ 为dispatcher模块创建线程管理对象
-- 已知成员:
--      .request    记录了http request路径的目录部分的table
--      .path       等同于.request
--      .urltoken   记录了http request路径的目录除外部分的table 
--      .tree       记录了节点树的root节点,也可以看作表整棵节点树
]]--
context = util.threadlocal()
uci = require "luci.model.uci"
i18n = require "luci.i18n"
_M.fs = fs

authenticator = {}

-- Index table
local index = nil   -- 这张index表中记录了所有controller目录下的lua文件中的index函数

-- Fastindex
local fi


--- Build the URL relative to the server webroot from given virtual path.
-- @param ...	Virtual path
-- @return 		Relative URL
function build_url(...)
	local path = {...}
	local url = { http.getenv("SCRIPT_NAME") or "" }

	local k, v
	for k, v in pairs(context.urltoken) do
		url[#url+1] = "/;"
		url[#url+1] = http.urlencode(k)
		url[#url+1] = "="
		url[#url+1] = http.urlencode(v)
	end

	local p
	for _, p in ipairs(path) do
		if p:match("^[a-zA-Z0-9_%-%.%%/,;]+$") then
			url[#url+1] = "/"
			url[#url+1] = p
		end
	end

	return table.concat(url, "")
end

--- Check whether a dispatch node shall be visible
-- @param node	Dispatch node
-- @return		Boolean indicating whether the node should be visible
function node_visible(node)
   if node then
	  return not (
		 (not node.title or #node.title == 0) or
		 (not node.target or node.hidden == true) or
		 (type(node.target) == "table" and node.target.type == "firstchild" and
		  (type(node.nodes) ~= "table" or not next(node.nodes)))
	  )
   end
   return false
end

--- Return a sorted table of visible childs within a given node
-- @param node	Dispatch node
-- @return		Ordered table of child node names
function node_childs(node)
	local rv = { }
	if node then
		local k, v
		for k, v in util.spairs(node.nodes,
			function(a, b)
				return (node.nodes[a].order or 100)
				     < (node.nodes[b].order or 100)
			end)
		do
			if node_visible(v) then
				rv[#rv+1] = k
			end
		end
	end
	return rv
end


--- Send a 404 error code and render the "error404" template if available.
-- @param message	Custom error message (optional)
-- @return			false
function error404(message)
	luci.http.status(404, "Not Found")
	message = message or "Not Found"

	require("luci.template")
	if not luci.util.copcall(luci.template.render, "error404") then
		luci.http.prepare_content("text/plain")
		luci.http.write(message)
	end
	return false
end

--- Send a 500 error code and render the "error500" template if available.
-- @param message	Custom error message (optional)#
-- @return			false
function error500(message)
	luci.util.perror(message)
	if not context.template_header_sent then
		luci.http.status(500, "Internal Server Error")
		luci.http.prepare_content("text/plain")
		luci.http.write(message)
	else
		require("luci.template")
		if not luci.util.copcall(luci.template.render, "error500", {message=message}) then
			luci.http.prepare_content("text/plain")
			luci.http.write(message)
		end
	end
	return false
end

function authenticator.htmlauth(validator, accs, default)
	local user = luci.http.formvalue("username")
	local pass = luci.http.formvalue("password")

	if user and validator(user, pass) then
		return user
	end

	require("luci.i18n")
	require("luci.template")
	context.path = {}
	luci.template.render("sysauth", {duser=default, fuser=user})
	return false

end

--- Dispatch an HTTP request.
-- 协程主程序,用于调度一个http的request
-- @param request	LuCI HTTP Request object    实例化的http request对象
-- @prefix(可选)      记录了路径前缀的数组
function httpdispatch(request, prefix)
	luci.http.context.request = request     -- 将该http request对象记录到http模块的线程管理对象中

	local r = {}
	context.request = r
	context.urltoken = {}

	local pathinfo = http.urldecode(request:getenv("PATH_INFO") or "", true)    -- 从环境变量中获取路径名

    -- 如果传入了路径前缀,则拷贝到r中
	if prefix then
		for _, node in ipairs(prefix) do
			r[#r+1] = node
		end
	end

    -- 解析从环境变量中获取的路径名,将目录部分拷贝到r中,将剩余部分拷贝到context.urltoken中
	local tokensok = true
	for node in pathinfo:gmatch("[^/]+") do
		local tkey, tval
		if tokensok then
			tkey, tval = node:match(";(%w+)=([a-fA-F0-9]*)")
		end
		if tkey then
			context.urltoken[tkey] = tval
		else
			tokensok = false
			r[#r+1] = node
		end
	end

	local stat, err = util.coxpcall(function()
		dispatch(context.request)
	end, error500)

	luci.http.close()

	--context._disable_memtrace()
end

--- Dispatches a LuCI virtual path.
-- 调度一个luci虚拟路径
-- @param request	Virtual path    显然该虚拟路径只是目录部分
function dispatch(request)
	--context._disable_memtrace = require "luci.debug".trap_memtrace("l")
	local ctx = context
	ctx.path = request      -- 将传入的luci虚拟路径记录到dispatcher模块的线程管理块中

    -- 加载luci的UCI配置模块
	local conf = require "luci.config"
    -- 确保luci的UCI文件中存在名为main的section
	assert(conf.main,
		"/etc/config/luci seems to be corrupt, unable to find section 'main'")

    -- 获取luci配置的语言
	local lang = conf.main.lang or "auto"
	if lang == "auto" then
		local aclang = http.getenv("HTTP_ACCEPT_LANGUAGE") or ""
		for lpat in aclang:gmatch("[%w-]+") do
			lpat = lpat and lpat:gsub("-", "_")
			if conf.languages[lpat] then
				lang = lpat
				break
			end
		end
	end
    -- 加载语言库并设置成配置的语言
	require "luci.i18n".setlanguage(lang)

	local c = ctx.tree
	local stat
    -- 如果节点树还未创建过,则首先需要创建整棵节点树
	if not c then
		c = createtree()
	end

	local track = {}
	local args = {}
	ctx.args = args
	ctx.requestargs = ctx.requestargs or args
	local n
	local token = ctx.urltoken
	local preq = {}
	local freq = {}

	for i, s in ipairs(request) do
		preq[#preq+1] = s
		freq[#freq+1] = s
		c = c.nodes[s]
		n = i
		if not c then
			break
		end

		util.update(track, c)

		if c.leaf then
			break
		end
	end

	if c and c.leaf then
		for j=n+1, #request do
			args[#args+1] = request[j]
			freq[#freq+1] = request[j]
		end
	end

	ctx.requestpath = ctx.requestpath or freq
	ctx.path = preq

	if track.i18n then
		i18n.loadc(track.i18n)
	end

	-- Init template engine     以下部分主要是在初始化模板引擎
	if (c and c.index) or not track.notemplate then
		local tpl = require("luci.template")
		local media = track.mediaurlbase or luci.config.main.mediaurlbase
		if not pcall(tpl.Template, "themes/%s/header" % fs.basename(media)) then
			media = nil
			for name, theme in pairs(luci.config.themes) do
				if name:sub(1,1) ~= "." and pcall(tpl.Template,
				 "themes/%s/header" % fs.basename(theme)) then
					media = theme
				end
			end
			assert(media, "No valid theme found")
		end

		local function _ifattr(cond, key, val)
			if cond then
				local env = getfenv(3)
				local scope = (type(env.self) == "table") and env.self
				return string.format(
					' %s="%s"', tostring(key),
					luci.util.pcdata(tostring( val
					 or (type(env[key]) ~= "function" and env[key])
					 or (scope and type(scope[key]) ~= "function" and scope[key])
					 or "" ))
				)
			else
				return ''
			end
		end

		tpl.context.viewns = setmetatable({
		   write       = luci.http.write;
		   include     = function(name) tpl.Template(name):render(getfenv(2)) end;
		   translate   = i18n.translate;
		   translatef  = i18n.translatef;
		   export      = function(k, v) if tpl.context.viewns[k] == nil then tpl.context.viewns[k] = v end end;
		   striptags   = util.striptags;
		   pcdata      = util.pcdata;
		   media       = media;
		   theme       = fs.basename(media);
		   resource    = luci.config.main.resourcebase;
		   ifattr      = function(...) return _ifattr(...) end;
		   attr        = function(...) return _ifattr(true, ...) end;
		}, {__index=function(table, key)
			if key == "controller" then
				return build_url()
			elseif key == "REQUEST_URI" then
				return build_url(unpack(ctx.requestpath))
			else
				return rawget(table, key) or _G[key]
			end
		end})
	end

	track.dependent = (track.dependent ~= false)
	assert(not track.dependent or not track.auto,
		"Access Violation\nThe page at '" .. table.concat(request, "/") .. "/' " ..
		"has no parent node so the access to this location has been denied.\n" ..
		"This is a software bug, please report this message at " ..
		"http://luci.subsignal.org/trac/newticket"
	)

    -- 以下这部分主要是认证环节
	if track.sysauth then
		local sauth = require "luci.sauth"

		local authen = type(track.sysauth_authenticator) == "function"
		 and track.sysauth_authenticator
		 or authenticator[track.sysauth_authenticator]

		local def  = (type(track.sysauth) == "string") and track.sysauth
		local accs = def and {track.sysauth} or track.sysauth
		local sess = ctx.authsession
		local verifytoken = false
		if not sess then
			sess = luci.http.getcookie("sysauth")
			sess = sess and sess:match("^[a-f0-9]*$")
			verifytoken = true
		end

		local sdat = sauth.read(sess)
		local user

		if sdat then
			if not verifytoken or ctx.urltoken.stok == sdat.token then
				user = sdat.user
			end
		else
			local eu = http.getenv("HTTP_AUTH_USER")
			local ep = http.getenv("HTTP_AUTH_PASS")
			if eu and ep and luci.sys.user.checkpasswd(eu, ep) then
				authen = function() return eu end
			end
		end

		if not util.contains(accs, user) then
			if authen then
				ctx.urltoken.stok = nil
				local user, sess = authen(luci.sys.user.checkpasswd, accs, def)
				if not user or not util.contains(accs, user) then
					return
				else
					local sid = sess or luci.sys.uniqueid(16)
					if not sess then
						local token = luci.sys.uniqueid(16)
						sauth.reap()
						sauth.write(sid, {
							user=user,
							token=token,
							secret=luci.sys.uniqueid(16)
						})
						ctx.urltoken.stok = token
					end
					luci.http.header("Set-Cookie", "sysauth=" .. sid.."; path="..build_url())
					ctx.authsession = sid
					ctx.authuser = user
				end
			else
				luci.http.status(403, "Forbidden")
				return
			end
		else
			ctx.authsession = sess
			ctx.authuser = user
		end
	end

	if track.setgroup then
		luci.sys.process.setgroup(track.setgroup)
	end

	if track.setuser then
		luci.sys.process.setuser(track.setuser)
	end

    -- 最后部分主要是在处理该节点的target字段
	local target = nil
	if c then
		if type(c.target) == "function" then    -- 如果该节点target字段类型为function,则局部变量target就赋值为该节点的target字段
			target = c.target
		elseif type(c.target) == "table" then   -- 如果该节点target字段类型为table,则局部变量target就赋值为该table中的target字段
			target = c.target.target
		end
	end

	if c and (c.index or type(target) == "function") then
		ctx.dispatched = c
		ctx.requested = ctx.requested or ctx.dispatched
	end

	if c and c.index then
		local tpl = require "luci.template"

		if util.copcall(tpl.render, "indexer", {}) then
			return true
		end
	end

	if type(target) == "function" then  -- target字段类型为function的情况
        -- 首先是修改target函数的环境
		util.copcall(function()
			local oldenv = getfenv(target)          -- 记录下原本的环境
			local module = require(c.module)        -- 加载该节点moudle字段记录的模块
            -- 创建一个target函数要使用的临时环境,实际就是创建一张有__index元方法的空table
            -- 这个__index元方法就是,依次从临时环境->module->原环境中去获取变量值
			local env = setmetatable({}, {__index=
			function(tbl, key)
				return rawget(tbl, key) or module[key] or oldenv[key]
			end})
            -- 临时修改target函数的环境
			setfenv(target, env)
		end)

        -- 执行target函数
		local ok, err
		if type(c.target) == "table" then
			ok, err = util.copcall(target, c.target, unpack(args))
		else
			ok, err = util.copcall(target, unpack(args))
		end
		assert(ok,
		       "Failed to execute " .. (type(c.target) == "function" and "function" or c.target.type or "unknown") ..
		       " dispatcher target for entry '/" .. table.concat(request, "/") .. "'.\n" ..
		       "The called action terminated with an exception:\n" .. tostring(err or "(unknown)"))
	else                                -- target字段类型除了function的情况
		local root = node()
		if not root or not root.target then
			error404("No root node was registered, this usually happens if no module was installed.\n" ..
			         "Install luci-mod-admin-full and retry. " ..
			         "If the module is already installed, try removing the /tmp/luci-indexcache file.")
		else
			error404("No page is registered at '/" .. table.concat(request, "/") .. "'.\n" ..
			         "If this url belongs to an extension, make sure it is properly installed.\n" ..
			         "If the extension was recently installed, try removing the /tmp/luci-indexcache file.")
		end
	end
end

--- Generate the dispatching index using the best possible strategy.
-- 生成一张名为index的table,table中记录了所有controller目录下lua文件中的index函数
function createindex()
	local path = luci.util.libpath() .. "/controller/"
	local suff = { ".lua", ".lua.gz" }

    -- 遍历luci的controller目录下的lua文件,生成一张index的table,如果支持fastindex则优先使用该算法,否则使用plain算法
	if luci.util.copcall(require, "luci.fastindex") then
		createindex_fastindex(path, suff)
	else
		createindex_plain(path, suff)
	end
end

--- Generate the dispatching index using the fastindex C-indexer.
-- @param path		Controller base directory
-- @param suffixes	Controller file suffixes
function createindex_fastindex(path, suffixes)
	index = {}

	if not fi then
		fi = luci.fastindex.new("index")
		for _, suffix in ipairs(suffixes) do
			fi.add(path .. "*" .. suffix)
			fi.add(path .. "*/*" .. suffix)
		end
	end
	fi.scan()

	for k, v in pairs(fi.indexes) do
		index[v[2]] = v[1]
	end
end

--- Generate the dispatching index using the native file-cache based strategy.
-- @param path		Controller base directory
-- @param suffixes	Controller file suffixes
function createindex_plain(path, suffixes)
	local controllers = { }
	for _, suffix in ipairs(suffixes) do
		nixio.util.consume((fs.glob(path .. "*" .. suffix)), controllers)
		nixio.util.consume((fs.glob(path .. "*/*" .. suffix)), controllers)
	end

	if indexcache then
		local cachedate = fs.stat(indexcache, "mtime")
		if cachedate then
			local realdate = 0
			for _, obj in ipairs(controllers) do
				local omtime = fs.stat(obj, "mtime")
				realdate = (omtime and omtime > realdate) and omtime or realdate
			end

			if cachedate > realdate then
				assert(
					sys.process.info("uid") == fs.stat(indexcache, "uid")
					and fs.stat(indexcache, "modestr") == "rw-------",
					"Fatal: Indexcache is not sane!"
				)

				index = loadfile(indexcache)()
				return index
			end
		end
	end

	index = {}

	for i,c in ipairs(controllers) do
		local modname = "luci.controller." .. c:sub(#path+1, #c):gsub("/", ".")
		for _, suffix in ipairs(suffixes) do
			modname = modname:gsub(suffix.."$", "")
		end

		local mod = require(modname)
		assert(mod ~= true,
		       "Invalid controller file found\n" ..
		       "The file '" .. c .. "' contains an invalid module line.\n" ..
		       "Please verify whether the module name is set to '" .. modname ..
		       "' - It must correspond to the file path!")

		local idx = mod.index
		assert(type(idx) == "function",
		       "Invalid controller file found\n" ..
		       "The file '" .. c .. "' contains no index() function.\n" ..
		       "Please make sure that the controller contains a valid " ..
		       "index function and verify the spelling!")

		index[modname] = idx
	end

	if indexcache then
		local f = nixio.open(indexcache, "w", 600)
		f:writeall(util.get_bytecode(index))
		f:close()
	end
end

--- Create the dispatching tree from the index.
-- 创建一棵节点树
-- Build the index before if it does not exist yet.
function createtree()
    -- 如果index表还未创建过,则首先需要创建这张表
	if not index then
		createindex()
	end

	local ctx  = context
	local tree = {nodes={}, inreq=true}
	local modi = {}

	ctx.treecache = setmetatable({}, {__mode="v"})
	ctx.tree = tree
	ctx.modifiers = modi

	-- Load default translation
	require "luci.i18n".loadc("base")

    -- 创建一个环境table,访问该环境的行为都会重定向到dispatcher模块
	local scope = setmetatable({}, {__index = luci.dispatcher})

    -- 调用controller目录下每个lua文件中的index函数,来创建树的每个节点
	for k, v in pairs(index) do
        -- 环境中额外附加了_NAME字段
		scope._NAME = k
        -- 调用每个index函数前,都会先修改该index函数的环境
		setfenv(v, scope)
		v()
	end

	local function modisort(a,b)
		return modi[a].order < modi[b].order
	end

	for _, v in util.spairs(modi, modisort) do
		scope._NAME = v.module
		setfenv(v.func, scope)
		v.func()
	end

    -- 最后返回创建完毕的节点树
	return tree
end

--- Register a tree modifier.
-- @param	func	Modifier function
-- @param	order	Modifier order value (optional)
function modifier(func, order)
	context.modifiers[#context.modifiers+1] = {
		func = func,
		order = order or 0,
		module
			= getfenv(2)._NAME
	}
end

--- Clone a node of the dispatching tree to another position.
-- @param	path	Virtual path destination
-- @param	clone	Virtual path source
-- @param	title	Destination node title (optional)
-- @param	order	Destination node order value (optional)
-- @return			Dispatching tree node
function assign(path, clone, title, order)
	local obj  = node(unpack(path))
	obj.nodes  = nil
	obj.module = nil

	obj.title = title
	obj.order = order

	setmetatable(obj, {__index = _create_node(clone)})

	return obj
end

--- Create a new dispatching node and define common parameters.
-- 创建一个新的节点,并初始化一些基础参数
-- @param	path	Virtual path    文件路径名的table(也就是菜单入口)
-- @param	target	Target function to call when dispatched.    进入该节点时会执行到的回调函数 
-- @param	title	Destination node title                      节点标题
-- @param	order	Destination node order value (optional)     节点序号(可选)
-- @return			Dispatching tree node
function entry(path, target, title, order)
	local c = node(unpack(path))

	c.target = target
	c.title  = title
	c.order  = order
	c.module = getfenv(2)._NAME     -- 记录下该节点关联的模块路径名(在createtree中创建每个index函数的环境时设置)

	return c
end

--- Fetch or create a dispatching node without setting the target module or
-- enabling the node.
-- 获取或创建一个新的节点(不做额外操作)
-- @param	...		Virtual path
-- @return			Dispatching tree node
function get(...)
	return _create_node({...})
end

--- Fetch or create a new dispatching node.
-- 获取或创建一个新的节点(附加了额外操作)
-- @param	...		Virtual path    文件路径名
-- @return			Dispatching tree node
function node(...)
	local c = _create_node({...})

	c.module = getfenv(2)._NAME
	c.auto = nil

	return c
end

function _create_node(path)
	if #path == 0 then
		return context.tree
	end

	local name = table.concat(path, ".")
	local c = context.treecache[name]

	if not c then
		local last = table.remove(path)
		local parent = _create_node(path)

		c = {nodes={}, auto=true}
		-- the node is "in request" if the request path matches
		-- at least up to the length of the node path
		if parent.inreq and context.path[#path+1] == last then
		  c.inreq = true
		end
		parent.nodes[last] = c
		context.treecache[name] = c
	end
	return c
end

-- Subdispatchers --
-- 自动跳转到最低序号页面
function _firstchild()
   local path = { unpack(context.path) }
   local name = table.concat(path, ".")
   local node = context.treecache[name]

   local lowest
   if node and node.nodes and next(node.nodes) then
	  local k, v
	  for k, v in pairs(node.nodes) do
		 if not lowest or
			(v.order or 100) < (node.nodes[lowest].order or 100)
		 then
			lowest = k
		 end
	  end
   end

   assert(lowest ~= nil,
		  "The requested node contains no childs, unable to redispatch")

   path[#path+1] = lowest
   dispatch(path)
end

--- Alias the first (lowest order) page automatically
-- 返回一张table,预示跳转到最低序号页面
function firstchild()
   return { type = "firstchild", target = _firstchild }
end

--- Create a redirect to another dispatching node.
-- 创建一个重定向到指定节点的函数
-- @param	...		Virtual path destination
function alias(...)
	local req = {...}
	return function(...)
		for _, r in ipairs({...}) do
			req[#req+1] = r
		end

		dispatch(req)
	end
end

--- Rewrite the first x path values of the request.
-- @param	n		Number of path values to replace
-- @param	...		Virtual path to replace removed path values with
function rewrite(n, ...)
	local req = {...}
	return function(...)
		local dispatched = util.clone(context.dispatched)

		for i=1,n do
			table.remove(dispatched, 1)
		end

		for i, r in ipairs(req) do
			table.insert(dispatched, i, r)
		end

		for _, r in ipairs({...}) do
			dispatched[#dispatched+1] = r
		end

		dispatch(dispatched)
	end
end


local function _call(self, ...)
	local func = getfenv()[self.name]
	assert(func ~= nil,
	       'Cannot resolve function "' .. self.name .. '". Is it misspelled or local?')

	assert(type(func) == "function",
	       'The symbol "' .. self.name .. '" does not refer to a function but data ' ..
	       'of type "' .. type(func) .. '".')

	if #self.argv > 0 then
		return func(unpack(self.argv), ...)
	else
		return func(...)
	end
end

--- Create a function-call dispatching target.
-- @param	name	Target function of local controller
-- @param	...		Additional parameters passed to the function
function call(name, ...)
	return {type = "call", argv = {...}, name = name, target = _call}
end


local _template = function(self, ...)
	require "luci.template".render(self.view)
end

--- Create a template render dispatching target.
-- @param	name	Template to be rendered
function template(name)
	return {type = "template", view = name, target = _template}
end


local function _cbi(self, ...)
	local cbi = require "luci.cbi"
	local tpl = require "luci.template"
	local http = require "luci.http"

	local config = self.config or {}
	local maps = cbi.load(self.model, ...)

	local state = nil

	for i, res in ipairs(maps) do
		res.flow = config
		local cstate = res:parse()
		if cstate and (not state or cstate < state) then
			state = cstate
		end
	end

	local function _resolve_path(path)
		return type(path) == "table" and build_url(unpack(path)) or path
	end

	if config.on_valid_to and state and state > 0 and state < 2 then
		http.redirect(_resolve_path(config.on_valid_to))
		return
	end

	if config.on_changed_to and state and state > 1 then
		http.redirect(_resolve_path(config.on_changed_to))
		return
	end

	if config.on_success_to and state and state > 0 then
		http.redirect(_resolve_path(config.on_success_to))
		return
	end

	if config.state_handler then
		if not config.state_handler(state, maps) then
			return
		end
	end

	http.header("X-CBI-State", state or 0)

	if not config.noheader then
		tpl.render("cbi/header", {state = state})
	end

	local redirect
	local messages
	local applymap   = false
	local pageaction = true
	local parsechain = { }

	for i, res in ipairs(maps) do
		if res.apply_needed and res.parsechain then
			local c
			for _, c in ipairs(res.parsechain) do
				parsechain[#parsechain+1] = c
			end
			applymap = true
		end

		if res.redirect then
			redirect = redirect or res.redirect
		end

		if res.pageaction == false then
			pageaction = false
		end

		if res.message then
			messages = messages or { }
			messages[#messages+1] = res.message
		end
	end

	for i, res in ipairs(maps) do
		res:render({
			firstmap   = (i == 1),
			applymap   = applymap,
			redirect   = redirect,
			messages   = messages,
			pageaction = pageaction,
			parsechain = parsechain
		})
	end

	if not config.nofooter then
		tpl.render("cbi/footer", {
			flow       = config,
			pageaction = pageaction,
			redirect   = redirect,
			state      = state,
			autoapply  = config.autoapply
		})
	end
end

--- Create a CBI model dispatching target.
-- @param	model	CBI model to be rendered
function cbi(model, config)
	return {type = "cbi", config = config, model = model, target = _cbi}
end


local function _arcombine(self, ...)
	local argv = {...}
	local target = #argv > 0 and self.targets[2] or self.targets[1]
	setfenv(target.target, self.env)
	target:target(unpack(argv))
end

--- Create a combined dispatching target for non argv and argv requests.
-- @param trg1	Overview Target
-- @param trg2	Detail Target
function arcombine(trg1, trg2)
	return {type = "arcombine", env = getfenv(), target = _arcombine, targets = {trg1, trg2}}
end


local function _form(self, ...)
	local cbi = require "luci.cbi"
	local tpl = require "luci.template"
	local http = require "luci.http"

	local maps = luci.cbi.load(self.model, ...)
	local state = nil

	for i, res in ipairs(maps) do
		local cstate = res:parse()
		if cstate and (not state or cstate < state) then
			state = cstate
		end
	end

	http.header("X-CBI-State", state or 0)
	tpl.render("header")
	for i, res in ipairs(maps) do
		res:render()
	end
	tpl.render("footer")
end

--- Create a CBI form model dispatching target.
-- @param	model	CBI form model tpo be rendered
function form(model)
	return {type = "cbi", model = model, target = _form}
end

--- Access the luci.i18n translate() api.
-- @class  function
-- @name   translate
-- @param  text    Text to translate
translate = i18n.translate

--- No-op function used to mark translation entries for menu labels.
-- This function does not actually translate the given argument but
-- is used by build/i18n-scan.pl to find translatable entries.
function _(text)
	return text
end
