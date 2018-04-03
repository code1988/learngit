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

local config = require "luci.config"    -- 加载luci配置获取模块,后续直接根据参数名就能索引对应的参数值
local ccache = require "luci.ccache"    -- 加载luci缓存模块

-- 创建luci缓存加载模块
module "luci.cacheloader"

-- 如果luci的UCI配置文件中配置了缓存使能,则使能luci的缓存机制
if config.ccache and config.ccache.enable == "1" then
	ccache.cache_ondemand()
end
