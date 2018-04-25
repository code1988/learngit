/*
 * Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../uloop.h"
#include "../list.h"

struct lua_uloop_fd {
	struct uloop_fd fd;
	int r;
	int fd_r;
};

struct lua_uloop_timeout {
	struct uloop_timeout t;
	int r;
};

struct lua_uloop_process {
	struct uloop_process p;
	int r;
};

static lua_State *state;

// 定时器回调函数
static void ul_timer_cb(struct uloop_timeout *t)
{
	struct lua_uloop_timeout *tout = container_of(t, struct lua_uloop_timeout, t);

    // 将全局table中名为"__uloop_cb"的table压栈
	lua_getglobal(state, "__uloop_cb");
    // 根据事先记录的"引用"从名为"__uloop_cb"的table中索引对应的lua超时回调，然后将该回调压栈
	lua_rawgeti(state, -1, tout->r);
    // 删除栈中名为"__uloop_cb"的table
	lua_remove(state, -2);

    // 调用栈中的lua超时回调，无入参，无返回值
	lua_call(state, 0, 0);
}

// 开启/更新定时器
static int ul_timer_set(lua_State *L)
{
	struct lua_uloop_timeout *tout;
	double set;

    // 确保栈顶元素是个整形的超时值
	if (!lua_isnumber(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

    // 获取栈顶的超时值
	set = lua_tointeger(L, -1);
    // 获取栈底的lua_uloop_timeout结构
	tout = lua_touserdata(L, 1);

    // 开启或更新定时器
	uloop_timeout_set(&tout->t, set);

	return 1;
}

// 销毁定时器
static int ul_timer_free(lua_State *L)
{
	struct lua_uloop_timeout *tout = lua_touserdata(L, 1);
	
	uloop_timeout_cancel(&tout->t);
	
	/* obj.__index.__gc = nil , make sure executing only once*/
	lua_getfield(L, -1, "__index");
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_getglobal(state, "__uloop_cb");
	luaL_unref(state, -1, tout->r);

	return 1;
}

// 定义了操作定时器对象的 C<-->lua 接口映射表
static const luaL_Reg timer_m[] = {
	{ "set", ul_timer_set },
	{ "cancel", ul_timer_free },
	{ NULL, NULL }
};

/* 创建一个定时器
 * lua函数"uloop.timer"参数入栈情况：
 *          -2  指定了该定时器的lua超时回调(必须)
 *          -1  指定了该定时器的超时值(可选)
 */
static int ul_timer(lua_State *L)
{
	struct lua_uloop_timeout *tout;
	int set = 0;
	int ref;

    // 如果传入了超时值，则记录下来
	if (lua_isnumber(L, -1)) {
		set = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

    // lua超时回调函数必须传入
	if (!lua_isfunction(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

    /* 将全局table中名为"__uloop_cb"的table压栈
     * 此时的栈：   
     *          -1 名为"_uloop_cb"的table
     *          -2 lua超时回调
     */
	lua_getglobal(L, "__uloop_cb");
    /* 拷贝一份lua超时回调函数压栈
     * 此时的栈：
     *          -1 lua超时回调
     *          -2 "_uloop_cb"的值
     *          -3 lua超时回调
     */
	lua_pushvalue(L, -2);
    /* 在名为"__uloop_cb"的table中创建一个对超时回调的引用(这个过程中会弹出栈顶的超时回调)
     * 此时的栈：
     *          -1 "_uloop_cb"的值
     *          -2 lua超时回调
     */
	ref = luaL_ref(L, -2);

    /* 分配一块lua_uloop_timeout结构的内存，并作为userdata压栈，同时返回其地址
     * 此时的栈：
     *          -1 userdata(lua_uloop_timeout结构)
     *          -2 "_uloop_cb"的值
     *          -3 lua超时回调
     */
	tout = lua_newuserdata(L, sizeof(*tout));
    /* 创建一个新的空table(实际就是元表)并压栈，同时为该table预分配了2个元素的非数组空间
     * 此时的栈：
     *          -1 空table(后续用作元表)
     *          -2 userdata(lua_uloop_timeout结构)
     *          -3 "_uloop_cb"的值
     *          -4 lua超时回调
     */
	lua_createtable(L, 0, 2);
    /* 拷贝一份刚创建的table压栈
     * 此时的栈：
     *          -1 空table
     *          -2 空table(后续用作元表)
     *          -3 userdata(lua_uloop_timeout结构)
     *          -4 "_uloop_cb"的值
     *          -5 lua超时回调
     */
	lua_pushvalue(L, -1);
    /* 弹出栈顶的空table，赋给元表中的"__index"
     * 此时的栈：
     *          -1 元表(有成员__index = 空table)
     *          -2 userdata(lua_uloop_timeout结构)
     *          -3 "_uloop_cb"的值
     *          -4 lua超时回调
     */
	lua_setfield(L, -2, "__index");
    /* 将C函数ul_timer_free压栈
     * 此时的栈：
     *          -1 ul_timer_free 
     *          -2 元表(有成员__index = 空table)
     *          -3 userdata(lua_uloop_timeout结构)
     *          -4 "_uloop_cb"的值
     *          -5 lua超时回调
     */
	lua_pushcfunction(L, ul_timer_free);
    /* 弹出栈顶的ul_timer_free，赋给元表中的"__gc"
     * 此时的栈：
     *          -1 元表(有成员__index = 空table;__gc = ul_timer_free)
     *          -2 userdata(lua_uloop_timeout结构)
     *          -3 "_uloop_cb"的值
     *          -4 lua超时回调
     */
	lua_setfield(L, -2, "__gc");
    /* 拷贝一份元表压栈
     * 此时的栈：
     *          -1 元表(有成员__index = 空table;__gc = ul_timer_free)
     *          -2 元表(有成员__index = 空table;__gc = ul_timer_free)
     *          -3 userdata(lua_uloop_timeout结构)
     *          -4 "_uloop_cb"的值
     *          -5 lua超时回调
     */
	lua_pushvalue(L, -1);
    /* 弹出栈顶的元表，然后将其设置为userdata的元表
     * 此时的栈：
     *          -1 元表(有成员__index = 空table;__gc = ul_timer_free)
     *          -2 userdata(lua_uloop_timeout结构，关联了元表)
     *          -3 "_uloop_cb"的值
     *          -4 lua超时回调
     */
	lua_setmetatable(L, -3);
    /* 拷贝一份userdata对象压栈
     * 此时的栈：
     *          -1 userdata(lua_uloop_timeout结构，关联了元表)
     *          -2 元表(有成员__index = 空table;__gc = ul_timer_free)
     *          -3 userdata(lua_uloop_timeout结构，关联了元表)
     *          -4 "_uloop_cb"的值
     *          -5 lua超时回调
     */
	lua_pushvalue(L, -2);
    /* 向userdata对象中注册定时器操作接口
     * 此时的栈：
     *          -1 userdata(lua_uloop_timeout结构，关联了元表、包含定时器操作接口)
     *          -2 元表(有成员__index = 空table;__gc = ul_timer_free)
     *          -3 userdata(lua_uloop_timeout结构，关联了元表)
     *          -4 "_uloop_cb"的值
     *          -5 lua超时回调
     */
	luaI_openlib(L, NULL, timer_m, 1);
	lua_pushvalue(L, -2);

	memset(tout, 0, sizeof(*tout));

	tout->r = ref;  // 记录"引用"值，指向lua函数超时回调
	tout->t.cb = ul_timer_cb;   // 注册C函数超时回调
    // 如果设置了超时值，则开启该定时器
	if (set)
		uloop_timeout_set(&tout->t, set);

	return 1;
}

// 被监听的fd的回调函数
static void ul_ufd_cb(struct uloop_fd *fd, unsigned int events)
{
	struct lua_uloop_fd *ufd = container_of(fd, struct lua_uloop_fd, fd);

	lua_getglobal(state, "__uloop_cb");
	lua_rawgeti(state, -1, ufd->r);
	lua_remove(state, -2);

	/* push fd object */
	lua_getglobal(state, "__uloop_fds");
	lua_rawgeti(state, -1, ufd->fd_r);
	lua_remove(state, -2);

	/* push events */
	lua_pushinteger(state, events);
	lua_call(state, 2, 0);
}

// 获取需要被监听的fd
static int get_sock_fd(lua_State* L, int idx) {
	int fd;
	if(lua_isnumber(L, idx)) {
		fd = lua_tonumber(L, idx);
	} else {
		luaL_checktype(L, idx, LUA_TUSERDATA);
		lua_getfield(L, idx, "getfd");
		if(lua_isnil(L, -1))
			return luaL_error(L, "socket type missing 'getfd' method");
		lua_pushvalue(L, idx - 1);
		lua_call(L, 1, 1);
		fd = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}
	return fd;
}

// 销毁被监听的fd
static int ul_ufd_delete(lua_State *L)
{
	struct lua_uloop_fd *ufd = lua_touserdata(L, 1);
	
	uloop_fd_delete(&ufd->fd);

	/* obj.__index.__gc = nil , make sure executing only once*/
	lua_getfield(L, -1, "__index");
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_getglobal(state, "__uloop_cb");
	luaL_unref(state, -1, ufd->r);
	lua_remove(state, -1);

	lua_getglobal(state, "__uloop_fds");
	luaL_unref(state, -1, ufd->fd_r);
	lua_remove(state, -1);

	return 1;
}

static const luaL_Reg ufd_m[] = {
	{ "delete", ul_ufd_delete },
	{ NULL, NULL }
};

// 将需要监听的fd加入epool监听池中
static int ul_ufd_add(lua_State *L)
{
	struct lua_uloop_fd *ufd;
	int fd = 0;
	unsigned int flags = 0;
	int ref;
	int fd_ref;

	if (lua_isnumber(L, -1)) {
		flags = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

	if (!lua_isfunction(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	fd = get_sock_fd(L, -2);

	lua_getglobal(L, "__uloop_cb");
	lua_pushvalue(L, -2);
	ref = luaL_ref(L, -2);
	lua_pop(L, 1);

	lua_getglobal(L, "__uloop_fds");
	lua_pushvalue(L, -3);
	fd_ref = luaL_ref(L, -2);
	lua_pop(L, 1);

	ufd = lua_newuserdata(L, sizeof(*ufd));

	lua_createtable(L, 0, 2);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, ul_ufd_delete);
	lua_setfield(L, -2, "__gc");
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -3);
	lua_pushvalue(L, -2);
	luaI_openlib(L, NULL, ufd_m, 1);
	lua_pushvalue(L, -2);

	memset(ufd, 0, sizeof(*ufd));

	ufd->r = ref;
	ufd->fd.fd = fd;
	ufd->fd_r = fd_ref;
	ufd->fd.cb = ul_ufd_cb;
	if (flags)
		uloop_fd_add(&ufd->fd, flags);

	return 1;
}

static void ul_process_cb(struct uloop_process *p, int ret)
{
	struct lua_uloop_process *proc = container_of(p, struct lua_uloop_process, p);

	lua_getglobal(state, "__uloop_cb");
	lua_rawgeti(state, -1, proc->r);

	luaL_unref(state, -2, proc->r);
	lua_remove(state, -2);
	lua_pushinteger(state, ret >> 8);
	lua_call(state, 1, 0);
}

static int ul_process(lua_State *L)
{
	struct lua_uloop_process *proc;
	pid_t pid;
	int ref;

	if (!lua_isfunction(L, -1) || !lua_istable(L, -2) ||
			!lua_istable(L, -3) || !lua_isstring(L, -4)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

	pid = fork();

	if (pid == -1) {
		lua_pushstring(L, "failed to fork");
		lua_error(L);

		return 0;
	}

	if (pid == 0) {
		/* child */
		int argn = lua_objlen(L, -3);
		int envn = lua_objlen(L, -2);
		char** argp = malloc(sizeof(char*) * (argn + 2));
		char** envp = malloc(sizeof(char*) * envn + 1);
		int i = 1;

		argp[0] = (char*) lua_tostring(L, -4);
		for (i = 1; i <= argn; i++) {
			lua_rawgeti(L, -3, i);
			argp[i] = (char*) lua_tostring(L, -1);
			lua_pop(L, 1);
		}
		argp[i] = NULL;

		for (i = 1; i <= envn; i++) {
			lua_rawgeti(L, -2, i);
			envp[i - 1] = (char*) lua_tostring(L, -1);
			lua_pop(L, 1);
		}
		envp[i - 1] = NULL;

		execve(*argp, argp, envp);
		exit(-1);
	}

	lua_getglobal(L, "__uloop_cb");
	lua_pushvalue(L, -2);
	ref = luaL_ref(L, -2);

	proc = lua_newuserdata(L, sizeof(*proc));
	memset(proc, 0, sizeof(*proc));

	proc->r = ref;
	proc->p.pid = pid;
	proc->p.cb = ul_process_cb;
	uloop_process_add(&proc->p);

	return 1;
}

// 创建epool句柄
static int ul_init(lua_State *L)
{
	uloop_init();
	lua_pushboolean(L, 1);

	return 1;
}

// 死循环监听epool
static int ul_run(lua_State *L)
{
	uloop_run();
	lua_pushboolean(L, 1);

	return 1;
}

// 跳出死循环
static int ul_end(lua_State *L)
{
	uloop_end();
	return 1;
}

static luaL_reg uloop_func[] = {
	{"init", ul_init},
	{"run", ul_run},
	{"timer", ul_timer},
	{"process", ul_process},
	{"fd_add", ul_ufd_add},
	{"cancel", ul_end},
	{NULL, NULL},
};

/* avoid warnings about missing declarations */
int luaopen_uloop(lua_State *L);
int luaclose_uloop(lua_State *L);

// uloop模块注册到lua环境的入口函数
int luaopen_uloop(lua_State *L)
{
	state = L;

    // 创建一个名为"__uloop_cb"的新的空table(预分配1个元素的数组空间)，
	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__uloop_cb");

    // 创建一个名为"__uloop_fds"的新的空table(预分配1个元素的数组空间)，
	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__uloop_fds");

    // 创建一个名为"uloop"的table，并将uloop_func中的所有C函数注册进去
	luaL_openlib(L, "uloop", uloop_func, 0);
	lua_pushstring(L, "_VERSION");
	lua_pushstring(L, "1.0");
	lua_rawset(L, -3);

    /* 继续往名为"uloop"的table中添加元素:
     *          "ULOOP_READ"            = ULOOP_READ
     *          "ULOOP_WRITE"           = ULOOP_WRITE
     *          "ULOOP_EDGE_TRIGGER"    = ULOOP_EDGE_TRIGGER
     *          "ULOOP_BLOCKING"        = ULOOP_BLOCKING
     */
	lua_pushstring(L, "ULOOP_READ");
	lua_pushinteger(L, ULOOP_READ);
	lua_rawset(L, -3);

	lua_pushstring(L, "ULOOP_WRITE");
	lua_pushinteger(L, ULOOP_WRITE);
	lua_rawset(L, -3);

	lua_pushstring(L, "ULOOP_EDGE_TRIGGER");
	lua_pushinteger(L, ULOOP_EDGE_TRIGGER);
	lua_rawset(L, -3);

	lua_pushstring(L, "ULOOP_BLOCKING");
	lua_pushinteger(L, ULOOP_BLOCKING);
	lua_rawset(L, -3);

	return 1;
}

int luaclose_uloop(lua_State *L)
{
	lua_pushstring(L, "Called");

	return 1;
}
