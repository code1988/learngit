--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id$
]]--

module("luci.controller.admin.index", package.seeall)

-- 创建root节点和admin系列节点(在createtree时被调用,本函数运行在一个特殊的环境)
function index()
    -- 首先创建一个root节点
	local root = node()
    -- 创建一个将root节点重定向到admin节点的函数
	if not root.target then
		root.target = alias("admin")
		root.index = true
	end

    -- 接着创建一个admin节点
	local page   = node("admin")
	page.target  = firstchild()     -- 进入admin节点时的行为是自动跳转到第一个子页面
	page.title   = _("Administration")
	page.order   = 10
	page.sysauth = "root"
	page.sysauth_authenticator = "htmlauth"
	page.ucidata = true
	page.index = true

	-- Empty services menu to be populated by addons
    -- 创建admin/services菜单,进入该菜单时的行为是自动跳转到第一个子页面
	entry({"admin", "services"}, firstchild(), _("Services"), 40).index = true
    -- 创建admin/logout菜单,进入该菜单时的行为是调用action_logout函数
	entry({"admin", "logout"}, call("action_logout"), _("Logout"), 90)
end

function action_logout()
	local dsp = require "luci.dispatcher"
	local sauth = require "luci.sauth"
	if dsp.context.authsession then
		sauth.kill(dsp.context.authsession)
		dsp.context.urltoken.stok = nil
	end

	luci.http.header("Set-Cookie", "sysauth=; path=" .. dsp.build_url())
	luci.http.redirect(luci.dispatcher.build_url())
end
