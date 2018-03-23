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
function p1(r)      -- 定义进程1终止时的回调函数
	print("Process 1 completed")
	print(r)
end

function p2(r)      -- 定义进程2终止时的回调函数
	print("Process 2 completed")
	print(r)
end

-- 创建一个定时器4,同时设置该定时器1000ms后超时
uloop.timer(
	function()  -- 定义了定时器4的超时回调函数:超时后创建一个进程并执行uloop_pid_test.sh
		uloop.process("uloop_pid_test.sh", {"foo", "bar"}, {"PROCESS=1"}, p1)
	end, 1000
)
-- 创建一个定时器5,同时设置该定时器2000ms后超时
uloop.timer(
	function()  -- 定义了定时器5的超时回调函数:超时后创建一个进程并执行uloop_pid_test.sh
		uloop.process("uloop_pid_test.sh", {"foo", "bar"}, {"PROCESS=2"}, p2)
	end, 2000
)

-- 将udp对象加入监听池,监听其读事件
udp_ev = uloop.fd_add(udp, function(ufd, events)    -- 定义了udp对象上有读事件时的回调函数
	local words, msg_or_ip, port_or_nil = ufd:receivefrom()
	print('Recv UDP packet from '..msg_or_ip..':'..port_or_nil..' : '..words)
	if words == "Stop!" then
		udp_ev:delete()
	end
end, uloop.ULOOP_READ)

udp_count = 0
-- 创建一个定时器6,同时设置该定时器3000ms后超时
udp_send_timer = uloop.timer(
	function()      -- 定义了定时器6的超时回调函数
		local s = socket.udp()  -- 动态创建一个udp对象
		local words
		if udp_count > 3 then   -- 第4次关闭定时器
			words = "Stop!"
			udp_send_timer:cancel()
		else                    -- 前3次设置定时器1000间隔超时
			words = 'Hello!'
			udp_send_timer:set(1000)
		end
        -- 往环回接口上的8080端口发udp包
		print('Send UDP packet to 127.0.0.1:8080 :'..words)
		s:sendto(words, '127.0.0.1', 8080)
		s:close()

		udp_count = udp_count + 1
	end, 3000
)

uloop.run()

