--[[
LuCI - SGI-Module for CGI

Description:
Server Gateway Interface for CGI

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
exectime = os.clock()

-- 创建了luci的服务器网关接口模块
module("luci.sgi.cgi", package.seeall)
local ltn12 = require("luci.ltn12")     -- 加载luci的网络库(其实就是luasocket库)
require("nixio.util")                   -- 加载lua在linux平台上的I/O库
require("luci.http")                    -- 加载luci的http库
require("luci.sys")                     -- 加载luci在linux平台上的系统库
require("luci.dispatcher")              -- 加载luci的调度库

-- Limited source to avoid endless blocking
-- 创建一个指定对象的受限的读操作(限制了读取的长度)
local function limitsource(handle, limit)
	limit = limit or 0
	local BLOCKSIZE = ltn12.BLOCKSIZE

	return function()
		if limit < 1 then
			handle:close()
			return nil
		else
			local read = (limit > BLOCKSIZE) and BLOCKSIZE or limit
			limit = limit - read

			local chunk = handle:read(read)
			if not chunk then handle:close() end
			return chunk
		end
	end
end

function run()
    -- 首先实例化一个http的request对象
	local r = luci.http.Request(
		luci.sys.getenv(),      -- 传入环境变量表
		limitsource(io.stdin, tonumber(luci.sys.getenv("CONTENT_LENGTH"))),     -- 创建对标准输入受限的读操作
		ltn12.sink.file(io.stderr)      -- 创建对标准出错的输出/结束封装
	)
	
    -- 接着创建一个协程,用作http服务调度
	local x = coroutine.create(luci.dispatcher.httpdispatch)
	local hcache = ""
	local active = true
	
    -- 最后进入循环,直到协程结束
	while coroutine.status(x) ~= "dead" do
        -- 唤醒协程,用于调度一次http的request
		local res, id, data1, data2 = coroutine.resume(x, r)

		if not res then
			print("Status: 500 Internal Server Error")
			print("Content-Type: text/plain\n")
			print(id)
			break;
		end

		if active then
			if id == 1 then
				io.write("Status: " .. tostring(data1) .. " " .. data2 .. "\r\n")
			elseif id == 2 then
				hcache = hcache .. data1 .. ": " .. data2 .. "\r\n"
			elseif id == 3 then
				io.write(hcache)
				io.write("\r\n")
			elseif id == 4 then
				io.write(tostring(data1 or ""))
			elseif id == 5 then
				io.flush()
				io.close()
				active = false
			elseif id == 6 then
				data1:copyz(nixio.stdout, data2)
				data1:close()
			end
		end
	end
end
