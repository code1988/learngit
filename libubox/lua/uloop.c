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

// 进一步封装的fd管理块
struct lua_uloop_fd {
	struct uloop_fd fd; // 面向C环境的fd管理块
	int r;              // lua环境中该fd监听事件触发回调的引用值
	int fd_r;
};

// 进一步封装的定时器管理块
struct lua_uloop_timeout {
	struct uloop_timeout t; // 面向C环境的定时器管理块
	int r;                  // lua环境中该定时器的超时回调的引用值
};

// 进一步封装的进程管理块
struct lua_uloop_process {
	struct uloop_process p; // 面向C环境的进程管理块
	int r;                  // lua环境中该进程的回调的引用值
};

static lua_State *state;

static void stack_dump(lua_State *L)
{
    int size = lua_gettop(L);
    printf("stack size = %d\n",size);
    int idx;
    for(idx = 1;idx <= size;idx++){
        int type = lua_type(L,idx);
        switch(type){
        case LUA_TBOOLEAN:
            printf("%d.boolean[%s]\n",idx,lua_toboolean(L,idx)?"true":"false");
            break;
        case LUA_TNUMBER:
            printf("%d.number[%d]\n",idx,(int)lua_tonumber(L,idx));
            break;
        case LUA_TSTRING:
            printf("%d.string[%s]\n",idx,lua_tostring(L,idx));
            break;
        default:
            printf("%d.%s\n",idx,lua_typename(L,type));
            break;
        }
    }
    printf("\n");
}

// C环境中的定时器超时回调函数
static void ul_timer_cb(struct uloop_timeout *t)
{
	struct lua_uloop_timeout *tout = container_of(t, struct lua_uloop_timeout, t);

    // 将全局table中名为"__uloop_cb"的table压栈
	lua_getglobal(state, "__uloop_cb");
    // 根据事先记录的"引用"在名为"__uloop_cb"的table中索引对应的lua环境中的超时回调，然后将该回调压栈
	lua_rawgeti(state, -1, tout->r);
    // 删除栈中名为"__uloop_cb"的table
	lua_remove(state, -2);

    // 调用栈中的lua环境中的超时回调，无入参，无返回值
	lua_call(state, 0, 0);
}

/* 开启/修改定时器超时值
 * 参数入栈情况：
 *          -1  指定了该定时器的超时值
 *          1   指定了该定时器userdata对象
 */
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

/* 关闭定时器超时
 * 参数入栈情况：
 *          1  指定了该定时器userdata对象
 */
static int ul_timer_free(lua_State *L)
{
    // 获取栈底的lua_uloop_timeout结构
	struct lua_uloop_timeout *tout = lua_touserdata(L, 1);
	
    // 关闭定时器超时
	uloop_timeout_cancel(&tout->t);
	
	/* obj.__index.__gc = nil , make sure executing only once*/
    // 将该userdata对象中索引为"__index"的值压栈,显然这会触发该userdata对象的元方法,最后实际压栈的就是元表自身
	lua_getfield(L, -1, "__index");
    // 以下3步操作其实就是清除元表中的"__gc"字段
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

    // 将全局table中名为"__uloop_cb"的table压栈
	lua_getglobal(state, "__uloop_cb");
    // 释放之前在名为"__uloop_cb"的table中创建的该定时器lua环境下超时回调
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
 *          2  指定了该定时器的超时值(可选)
 *          1  指定了该定时器的lua超时回调(必须)
 *
 * 最后返回给lua环境的是一个struct lua_uloop_timeout结构的userdata对象,该对象同时关联了元表
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
     *          -1 名为"__uloop_cb"的table
     *          -2 lua超时回调
     */
	lua_getglobal(L, "__uloop_cb");
    /* 拷贝一份lua超时回调函数压栈
     * 此时的栈：
     *          -1 lua超时回调
     *          -2 名为"__uloop_cb"的table
     *          -3 lua超时回调
     */
	lua_pushvalue(L, -2);
    /* 在名为"__uloop_cb"的table中创建一个对超时回调的引用(这个过程中会弹出栈顶的超时回调)
     * 此时的栈：
     *          -1 名为"__uloop_cb"的table
     *          -2 lua超时回调
     */
	ref = luaL_ref(L, -2);

    /* 分配一块lua_uloop_timeout结构的内存，并作为userdata压栈，同时返回其地址
     * 此时的栈：
     *          -1 userdata(lua_uloop_timeout结构)
     *          -2 名为"__uloop_cb"的table
     *          -3 lua超时回调
     */
	tout = lua_newuserdata(L, sizeof(*tout));
    /* 创建一个新的空table(实际就是元表)并压栈，同时为该table预分配了2个元素的非数组空间
     * 此时的栈：
     *          -1 空table(后续用作元表)
     *          -2 userdata(lua_uloop_timeout结构)
     *          -3 名为"__uloop_cb"的table
     *          -4 lua超时回调
     */
	lua_createtable(L, 0, 2);
    /* 拷贝一份刚创建的table压栈
     * 此时的栈：
     *          -1 空table
     *          -2 空table(后续用作元表)
     *          -3 userdata(lua_uloop_timeout结构)
     *          -4 名为"__uloop_cb"的table
     *          -5 lua超时回调
     */
	lua_pushvalue(L, -1);
    /* 弹出栈顶的空table，赋给自身中的"__index"
     * 此时的栈：
     *          -1 元表(有成员__index = 自身table)
     *          -2 userdata(lua_uloop_timeout结构)
     *          -3 名为"__uloop_cb"的table
     *          -4 lua超时回调
     */
	lua_setfield(L, -2, "__index");
    /* 将C函数ul_timer_free压栈
     * 此时的栈：
     *          -1 ul_timer_free 
     *          -2 元表(有成员__index = 自身table)
     *          -3 userdata(lua_uloop_timeout结构)
     *          -4 名为"__uloop_cb"的table
     *          -5 lua超时回调
     */
	lua_pushcfunction(L, ul_timer_free);
    /* 弹出栈顶的ul_timer_free，赋给table中的"__gc"
     * 此时的栈：
     *          -1 元表(有成员__index = 自身table;__gc = ul_timer_free)
     *          -2 userdata(lua_uloop_timeout结构)
     *          -3 名为"__uloop_cb"的table
     *          -4 lua超时回调
     */
	lua_setfield(L, -2, "__gc");
    /* 拷贝一份table压栈
     * 此时的栈：
     *          -1 元表(有成员__index = 自身table;__gc = ul_timer_free)
     *          -2 元表(有成员__index = 自身table;__gc = ul_timer_free)
     *          -3 userdata(lua_uloop_timeout结构)
     *          -4 名为"__uloop_cb"的table
     *          -5 lua超时回调
     */
	lua_pushvalue(L, -1);
    /* 弹出栈顶的table，然后将其设置为userdata的元表
     * 此时的栈：
     *          -1 元表(有成员__index = 自身table;__gc = ul_timer_free)
     *          -2 userdata(lua_uloop_timeout结构，关联了元表)
     *          -3 名为"__uloop_cb"的table
     *          -4 lua超时回调
     */
	lua_setmetatable(L, -3);
    /* 拷贝一份userdata对象压栈
     * 此时的栈：
     *          -1 userdata(lua_uloop_timeout结构，关联了元表)
     *          -2 元表(有成员__index = 自身table;__gc = ul_timer_free)
     *          -3 userdata(lua_uloop_timeout结构，关联了元表)
     *          -4 名为"__uloop_cb"的table
     *          -5 lua超时回调
     */
	lua_pushvalue(L, -2);
    /* 向元表中注册C闭包形式的定时器操作接口(upvalue就是userdata)
     * 此时的栈：
     *          -1 元表(有成员__index = 自身table;__gc = ul_timer_free;C闭包形式的定时器操作接口)
     *          -2 userdata(lua_uloop_timeout结构，关联了元表)
     *          -3 名为"__uloop_cb"的table
     *          -4 lua超时回调
     */
	luaI_openlib(L, NULL, timer_m, 1);
    /* 拷贝一份userdata对象压栈(将作为返回值返回给lua环境)
     * 此时的栈:
     *          -1 userdata(lua_uloop_timeout结构，关联了元表)
     *          -2 元表(有成员__index = 自身table;__gc = ul_timer_free;C闭包形式的定时器操作接口)
     *          -3 userdata(lua_uloop_timeout结构，关联了元表)
     *          -4 名为"__uloop_cb"的table
     *          -5 lua超时回调
     */
	lua_pushvalue(L, -2);

	memset(tout, 0, sizeof(*tout));

	tout->r = ref;  // 记录lua环境中该定时器超时回调的"引用"值
	tout->t.cb = ul_timer_cb;   // 注册C环境中的超时回调
    // 如果设置了超时值，则开启该定时器
	if (set)
		uloop_timeout_set(&tout->t, set);

    // 返回给lua环境1个返回值(位于栈顶)
	return 1;
}

// C环境下被监听的fd有事件触发后的回调函数
static void ul_ufd_cb(struct uloop_fd *fd, unsigned int events)
{
	struct lua_uloop_fd *ufd = container_of(fd, struct lua_uloop_fd, fd);

    // 将全局环境中名为"__uloop_cb"的table压栈
	lua_getglobal(state, "__uloop_cb");
    // 根据之前记录的引用值,从名为"__uloop_cb"的table中获取lua环境下对应的回调函数
	lua_rawgeti(state, -1, ufd->r);
    // 移除栈中名为"__uloop_cb"的table
	lua_remove(state, -2);

	/* push fd object */
    // 将全局环境中名为"__uloop_fds"的table压栈
	lua_getglobal(state, "__uloop_fds");
    // 根据之前记录的引用值,从名为"__uloop_fds"的table中获取对应的包含fd信息的自定义lua对象
	lua_rawgeti(state, -1, ufd->fd_r);
    // 移除栈中名为"__uloop_fds"的table
	lua_remove(state, -2);

	/* push events */
    // 将触发的事件类型压入栈顶
	lua_pushinteger(state, events);
    // 最后执行lua环境下该fd事件触发后的回调函数(2个入参:包含fd信息的自定义lua对象,触发的事件类型)
	lua_call(state, 2, 0);
}

// 从栈中idx索引处的自定义对象中获取需要被监听的fd
static int get_sock_fd(lua_State* L, int idx) {
	int fd;
    // 如果该索引处是一个number类型的lua值,意味着就是fd
	if(lua_isnumber(L, idx)) {
		fd = lua_tonumber(L, idx);
	} else {
        // 如果不是number类型,则必须是userdata类型(否则抛出错误)
		luaL_checktype(L, idx, LUA_TUSERDATA);
        // 从该userdata类型的lua对象中获取getfd方法并压入栈顶
		lua_getfield(L, idx, "getfd");
		if(lua_isnil(L, -1))
			return luaL_error(L, "socket type missing 'getfd' method");
        // 对该userdata类型的lua对象做一份拷贝并压入栈顶
		lua_pushvalue(L, idx - 1);
        // 调用getfd方法,入参就是userdata类型的lua对象,返回值就是fd
		lua_call(L, 1, 1);
        // 获取栈顶的返回值,也就是fd
		fd = lua_tointeger(L, -1);
        // 弹出栈顶的fd
		lua_pop(L, 1);
	}
	return fd;
}

/* 移除监听池中的指定fd
 * 参数入栈情况：
 *          1  指定了该fd监听事件的userdata对象
 */
static int ul_ufd_delete(lua_State *L)
{
    // 获取栈底的lua_uloop_fd结构
	struct lua_uloop_fd *ufd = lua_touserdata(L, 1);
	
    // 删除该fd控制块
	uloop_fd_delete(&ufd->fd);

	/* obj.__index.__gc = nil , make sure executing only once*/
    // 将该userdata对象中索引为"__index"的值压栈,显然这会触发该userdata对象的元方法,最后实际压栈的就是元表自身
	lua_getfield(L, -1, "__index");
    // 以下3步操作其实就是清除元表中的"__gc"字段
	lua_pushstring(L, "__gc");
	lua_pushnil(L);
	lua_settable(L, -3);

    // 将全局table中名为"__uloop_cb"的table压栈
	lua_getglobal(state, "__uloop_cb");
    // 释放之前在名为"__uloop_cb"的table中创建的lua环境下该fd事件触发回调
	luaL_unref(state, -1, ufd->r);
    // 移除栈中名为"__uloop_cb"的table
	lua_remove(state, -1);

    // 将全局table中名为"__uloop_fds"的table压栈
	lua_getglobal(state, "__uloop_fds");
    // 释放之前在名为"__uloop_cb"的table中创建的自定义的包含该fd的lua对象
	luaL_unref(state, -1, ufd->fd_r);
    // 移除栈中名为"__uloop_fds"的table
	lua_remove(state, -1);

	return 1;
}

// 定义了操作自定义fd对象的 C<-->lua 接口映射表
static const luaL_Reg ufd_m[] = {
	{ "delete", ul_ufd_delete },
	{ NULL, NULL }
};

/* 将需要监听的fd加入epool监听池中
 * lua函数"uloop.fd_add"参数入栈情况：
 *          -1  指定了该fd上需要监听的事件(可选)
 *          -2  指定了该fd上监听事件触发后lua环境下的回调函数
 *          -3  指定了自定义的包含了该fd的lua对象,该对象被限定为2种类型:
 *                          如果是number类型时,就是直接传入fd
 *                          如果是userdata类型时,必须包含getfd方法
 *
 * 最后返回给lua环境的是一个struct lua_uloop_fd结构的userdata对象,该对象同时关联了元表
 */
static int ul_ufd_add(lua_State *L)
{
	struct lua_uloop_fd *ufd;
	int fd = 0;
	unsigned int flags = 0;
	int ref;
	int fd_ref;

    // 如果栈顶是一个number类型的lua对象,则意味着传入了要监听的事件类型,将其记录然后弹出栈
	if (lua_isnumber(L, -1)) {
		flags = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

    // 此时的栈顶必须是一个函数
	if (!lua_isfunction(L, -1)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

    // 从栈中自定义的lua对象中获取fd
	fd = get_sock_fd(L, -2);

    // 将全局环境中名为"__uloop_cb"的table压栈
	lua_getglobal(L, "__uloop_cb");
    // 对事件触发后lua环境下的回调函数做一份拷贝并压入栈顶
	lua_pushvalue(L, -2);
    // 在名为"__uloop_cb"的table中创建一个对事件触发后lua环境下回调函数的引用(这个过程中会弹出栈顶的回调函数)
	ref = luaL_ref(L, -2);
    // 弹出名为"__uloop_cb"的table
	lua_pop(L, 1);

    // 将全局环境中名为"__uloop_fds"的table压栈
	lua_getglobal(L, "__uloop_fds");
    // 对自定义的lua对象做一份拷贝并压入栈顶
	lua_pushvalue(L, -3);
    // 在名为"__uloop_fds"的table中创建一个对自定义lua对象的引用(这个过程中会弹出栈顶的自定义lua对象)
	fd_ref = luaL_ref(L, -2);
    // 弹出名为"__uloop_fds"的table
	lua_pop(L, 1);

    // 分配一块lua_uloop_fd结构的内存，并作为userdata对象压栈，同时返回其地址
	ufd = lua_newuserdata(L, sizeof(*ufd));

    // 创建一个新的空table(后续实际将作为元表)并压栈，同时为该table预分配了2个元素的非数组空间
	lua_createtable(L, 0, 2);
    // 拷贝一份刚创建的table压栈
	lua_pushvalue(L, -1);
    // 弹出栈顶的空table，赋给自身中的"__index"
	lua_setfield(L, -2, "__index");
    // 将C函数ul_ufd_delete压栈
	lua_pushcfunction(L, ul_ufd_delete);
    // 弹出栈顶的ul_ufd_delete，赋给table中的"__gc"
	lua_setfield(L, -2, "__gc");
    // 拷贝一份table压栈,显然此时该table中已经有了"__index"和"__gc"两个成员
	lua_pushvalue(L, -1);
    // 弹出栈顶的table，然后将其设置为userdata对象的元表
	lua_setmetatable(L, -3);
    // 拷贝一份userdata对象压栈
	lua_pushvalue(L, -2);
    // 向元表中注册C闭包形式的fd操作接口(upvalue就是userdata)
    // 至此,元表中有成员: __index = 自身table;__gc = ul_timer_free;C闭包形式的fd操作接口
	luaI_openlib(L, NULL, ufd_m, 1);
    // 拷贝一份userdata对象压栈(将作为返回值返回给lua环境)
	lua_pushvalue(L, -2);

	memset(ufd, 0, sizeof(*ufd));

	ufd->r = ref;           // 记录lua环境中该fd监听事件触发后lua环境下回调函数的引用值
	ufd->fd.fd = fd;
	ufd->fd_r = fd_ref;     // 记录自定义的包含了该fd的lua对象的引用值
	ufd->fd.cb = ul_ufd_cb; // 记录C环境下该fd监听事件触发后的回调函数

    // 如果要监听的事件有效,就将该fd加入监听池
	if (flags)
		uloop_fd_add(&ufd->fd, flags);

    // 返回给lua环境1个返回值(位于栈顶)
	return 1;
}

// C环境中的进程终止回调函数
static void ul_process_cb(struct uloop_process *p, int ret)
{
	struct lua_uloop_process *proc = container_of(p, struct lua_uloop_process, p);

    // 将全局table中名为"__uloop_cb"的table压栈
	lua_getglobal(state, "__uloop_cb");
    // 根据事先记录的"引用"在名为"__uloop_cb"的table中索引对应的lua环境中的进程终止回调，然后将该回调压栈
	lua_rawgeti(state, -1, proc->r);

    // 释放之前在名为"__uloop_cb"的table中创建的lua环境下进程终止回调
	luaL_unref(state, -2, proc->r);
    // 移除栈中的名为"__uloop_cb"的table
	lua_remove(state, -2);
    // 往栈顶压入返回值
	lua_pushinteger(state, ret >> 8);
    // 调用lua环境中的进程终止回调
	lua_call(state, 1, 0);
}

/* 创建一个进程
 * lua函数"uloop.process"参数入栈情况：
 *          -1  指定了该进程终止时的回调函数
 *          -2  指定了要执行程序的环境变量表
 *          -3  指定了要执行程序的命令行参数表
 *          -4  指定了该进程要执行的程序
 *
 * 最后返回给lua环境的是一个struct lua_uloop_process结构的userdata对象
 */
static int ul_process(lua_State *L)
{
	struct lua_uloop_process *proc;
	pid_t pid;
	int ref;

    // 首先检查入参是否正确
	if (!lua_isfunction(L, -1) || !lua_istable(L, -2) ||
			!lua_istable(L, -3) || !lua_isstring(L, -4)) {
		lua_pushstring(L, "invalid arg list");
		lua_error(L);

		return 0;
	}

    // 创建子进程
	pid = fork();

	if (pid == -1) {
		lua_pushstring(L, "failed to fork");
		lua_error(L);

		return 0;
	}

	if (pid == 0) {
		/* child */
        // 子进程首先从栈中提取要执行的程序 命令行参数表 环境变量表
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

        // 执行程序
		execve(*argp, argp, envp);
		exit(-1);
	}

    /* 父进程将全局table中名为"__uloop_cb"的table压栈
     * 此时的栈：   
     *          -1  名为"__uloop_cb"的table
     *          -2  进程终止时的回调函数
     *          ... 
     */
	lua_getglobal(L, "__uloop_cb");
    /* 拷贝一份进程终止时的回调函数压栈
     * 此时的栈：
     *          -1  进程终止时的回调函数
     *          -2  名为"__uloop_cb"的table
     *          -3  进程终止时的回调函数
     *          ...
     */
	lua_pushvalue(L, -2);
    /* 在名为"__uloop_cb"的table中创建一个对进程终止回调的引用(这个过程中会弹出栈顶的进程终止回调)
     * 此时的栈：
     *          -1  名为"__uloop_cb"的table
     *          -2  进程终止时的回调函数
     */
	ref = luaL_ref(L, -2);

    /* 分配一块lua_uloop_process结构的内存，并作为userdata压栈，同时返回其地址
     * 此时的栈：
     *          -1  userdata(lua_uloop_process结构)
     *          -2  名为"__uloop_cb"的table
     *          -3  进程终止时的回调函数
     */
	proc = lua_newuserdata(L, sizeof(*proc));
	memset(proc, 0, sizeof(*proc));

	proc->r = ref;                  // 记录lua环境中该进程终止回调的"引用"值
	proc->p.pid = pid;              // 记录该进程的pid
	proc->p.cb = ul_process_cb;     // 注册C环境中该进程终止回调函数
	uloop_process_add(&proc->p);    // 最后将该进程管理块注册到uloop中

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
    // 首先记录下lua状态机
	state = L;

    // 创建一个名为"__uloop_cb"的新的空table(预分配1个元素的数组空间)，
    // 这个table后续主要用来存放lua环境下的定时器超时回调/进程终止回调/监听事件触发回调
	lua_createtable(L, 1, 0);
	lua_setglobal(L, "__uloop_cb");

    // 创建一个名为"__uloop_fds"的新的空table(预分配1个元素的数组空间)，
    // 这个table后续主要用来存放lua环境下包含了fd信息的自定义lua对象
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
