#!/usr/bin/env lua

local socket = require "socket"
local uloop = require("uloop")
uloop.init()        -- 创建epoll对象

local udp = socket.udp()    -- 创建一个udp对象
udp:settimeout(0)
udp:setsockname('*', 8080)  -- 将该udp对象绑定在8080端口上

-- timer example 1
local timer
function t()                -- 定义一个定时器回调函数(间隔1000ms)
	print("1000 ms timer run");
	timer:set(1000)
end
timer = uloop.timer(t)      -- 创建一个定时器1
timer:set(1000)             -- 设置该定时器1000ms后超时(注意":"意味着会传入该定时器对象本身作为第一个入参)

-- timer example 2
uloop.timer(function() print("2000 ms timer run"); end, 2000)   -- 创建一个定时器2,同时设置该定时器2000ms后超时

-- timer example 3
uloop.timer(function() print("3000 ms timer run"); end, 3000):cancel()  -- 创建一个定时器3,同时设置该定时器3000ms后超时,紧接着取消该定时器的超时设置

-- process
function p1(r)      -- 定义一个进程1回调函数
	print("Process 1 completed")
	print(r)
end

function p2(r)      -- 定义一个进程2回调函数
	print("Process 2 completed")
	print(r)
end

uloop.timer(
	function()
		uloop.process("uloop_pid_test.sh", {"foo", "bar"}, {"PROCESS=1"}, p1)
	end, 1000
)
uloop.timer(
	function()
		uloop.process("uloop_pid_test.sh", {"foo", "bar"}, {"PROCESS=2"}, p2)
	end, 2000
)

udp_ev = uloop.fd_add(udp, function(ufd, events)
	local words, msg_or_ip, port_or_nil = ufd:receivefrom()
	print('Recv UDP packet from '..msg_or_ip..':'..port_or_nil..' : '..words)
	if words == "Stop!" then
		udp_ev:delete()
	end
end, uloop.ULOOP_READ)

udp_count = 0
udp_send_timer = uloop.timer(
	function()
		local s = socket.udp()
		local words
		if udp_count > 3 then
			words = "Stop!"
			udp_send_timer:cancel()
		else
			words = 'Hello!'
			udp_send_timer:set(1000)
		end
		print('Send UDP packet to 127.0.0.1:8080 :'..words)
		s:sendto(words, '127.0.0.1', 8080)
		s:close()

		udp_count = udp_count + 1
	end, 3000
)

uloop.run()

