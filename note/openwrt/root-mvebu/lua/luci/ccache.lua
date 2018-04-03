--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

local io = require "io"
local fs = require "nixio.fs"
local util = require "luci.util"
local nixio = require "nixio"
local debug = require "debug"
local string = require "string"
local package = require "package"

local type, loadfile = type, loadfile


-- 创建luci的C文件缓存模块
module "luci.ccache"

-- 使能luci的缓存功能(带封装)
function cache_ondemand(...)
    -- 如果当前函数所在文件路径有效,则使能缓存功能
	if debug.getinfo(1, 'S').source ~= "=?" then
		cache_enable(...)
	end
end

-- 使能luci的缓存功能,实质就是重定义了C文件加载器
function cache_enable(cachepath, mode)
	cachepath = cachepath or "/tmp/luci-modulecache"    -- 定义了缺省缓存目录路径
	mode = mode or "r--r--r--"                          -- 定义了缺省缓存文件权限

	local loader = package.loaders[2]   -- 记录默认的C文件加载器
	local uid    = nixio.getuid()

    -- 如果缓存目录不存在,则创建该目录
	if not fs.stat(cachepath) then
		fs.mkdir(cachepath)
	end

	local function _encode_filename(name)
		local encoded = ""
		for i=1, #name do
			encoded = encoded .. ("%2X" % string.byte(name, i))
		end
		return encoded
	end

	local function _load_sane(file)
		local stat = fs.stat(file)
		if stat and stat.uid == uid and stat.modestr == mode then
			return loadfile(file)
		end
	end

	local function _write_sane(file, func)
		if nixio.getuid() == uid then
			local fp = io.open(file, "w")
			if fp then
				fp:write(util.get_bytecode(func))
				fp:close()
				fs.chmod(file, mode)
			end
		end
	end

    -- 重定义C文件加载器
	package.loaders[2] = function(mod)
		local encoded = cachepath .. "/" .. _encode_filename(mod)   -- 将传入的模块名进行编码,生成对应的文件名
		local modcons = _load_sane(encoded)                         -- 加载编码后的文件名
		
        -- 如果加载成功,意味着该缓存文件之前有过加载
		if modcons then
			return modcons
		end

		-- No cachefile
        -- 如果加载失败,意味着该缓存文件之前没有加载过,这里调用默认的加载器进行加载
		modcons = loader(mod)
		if type(modcons) == "function" then
			_write_sane(encoded, modcons)
		end
		return modcons
	end
end
