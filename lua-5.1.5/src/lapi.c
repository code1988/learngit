/*
** $Id: lapi.c,v 2.55.1.5 2008/07/04 18:41:18 roberto Exp $
** Lua API

定义了lua对外的基础API

** See Copyright Notice in lua.h
*/


#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#define lapi_c
#define LUA_CORE

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"



const char lua_ident[] =
  "$Lua: " LUA_RELEASE " " LUA_COPYRIGHT " $\n"
  "$Authors: " LUA_AUTHORS " $\n"
  "$URL: www.lua.org $\n";



#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->base))

#define api_checkvalidindex(L, i)	api_check(L, (i) != luaO_nilobject)

#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}


// 返回栈中对应索引号的元素
static TValue *index2adr (lua_State *L, int idx) 
{
    if (idx > 0)    // 正数索引
    {
        // 从栈底开始索引,起始索引号1
        TValue *o = L->base + (idx - 1);
        api_check(L, idx <= L->ci->top - L->base);
        if (o >= L->top) return cast(TValue *, luaO_nilobject);
        else return o;
    }
    else if (idx > LUA_REGISTRYINDEX)   // 负数索引
    {
        api_check(L, idx != 0 && -idx <= L->top - L->base);
        // 从栈顶开始索引，起始索引号-1
        return L->top + idx;
    }
    else                    // 一些特定的索引号 
        switch (idx) 
        {  /* pseudo-indices */
            case LUA_REGISTRYINDEX: return registry(L);
            case LUA_ENVIRONINDEX: 
            {
                Closure *func = curr_func(L);
                sethvalue(L, &L->env, func->c.env);
                return &L->env;
            }
            case LUA_GLOBALSINDEX: return gt(L);
            default: 
            {
                Closure *func = curr_func(L);
                idx = LUA_GLOBALSINDEX - idx;
                return (idx <= func->c.nupvalues) ? &func->c.upvalue[idx-1] : cast(TValue *, luaO_nilobject);
            }
        }
}

// 获取当前的环境
static Table *getcurrenv (lua_State *L) {
  if (L->ci == L->base_ci)  /* no enclosing function? */
    return hvalue(gt(L));  /* use global table as environment */
  else {
    Closure *func = curr_func(L);
    return func->c.env;
  }
}


void luaA_pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top, o);
  api_incr_top(L);
}

// 检查lua栈中是否有足够的空间
LUA_API int lua_checkstack (lua_State *L, int size) {
  int res = 1;
  lua_lock(L);
  if (size > LUAI_MAXCSTACK || (L->top - L->base + size) > LUAI_MAXCSTACK)
    res = 0;  /* stack overflow */
  else if (size > 0) {
    luaD_checkstack(L, size);
    if (L->ci->top < L->top + size)
      L->ci->top = L->top + size;
  }
  lua_unlock(L);
  return res;
}


LUA_API void lua_xmove (lua_State *from, lua_State *to, int n) {
  int i;
  if (from == to) return;
  lua_lock(to);
  api_checknelems(from, n);
  api_check(from, G(from) == G(to));
  api_check(from, to->ci->top - to->top >= n);
  from->top -= n;
  for (i = 0; i < n; i++) {
    setobj2s(to, to->top++, from->top + i);
  }
  lua_unlock(to);
}


LUA_API void lua_setlevel (lua_State *from, lua_State *to) {
  to->nCcalls = from->nCcalls;
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  return old;
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  L1 = luaE_newthread(L);
  setthvalue(L, L->top, L1);
  api_incr_top(L);
  lua_unlock(L);
  luai_userstatethread(L, L1);
  return L1;
}



/*
** basic stack manipulation

以下都是针对堆栈的基本操作API
*/

/* 返回栈顶元素的正索引，也就是获取栈中元素的个数
 *
 * 备注：返回0意味着堆栈为空
 */
LUA_API int lua_gettop (lua_State *L) {
  return cast_int(L->top - L->base);
}

/* 将栈顶设置为一个指定位置，即修改栈中元素的数量
 *
 * @idx - 正数时，如果比之前的栈顶高，会向栈中压入nil来补足大小，如果比之前的栈顶低，意味着多出来的元素会被丢弃;
 *        负数时，只有丢弃功能
 */
LUA_API void lua_settop (lua_State *L, int idx) {
  lua_lock(L);
  if (idx >= 0) {
    api_check(L, idx <= L->stack_last - L->base);
    while (L->top < L->base + idx)
      setnilvalue(L->top++);
    L->top = L->base + idx;
  }
  else {
    api_check(L, -(idx+1) <= (L->top - L->base));
    L->top += idx+1;  /* `subtract' index (index is negative) */
  }
  lua_unlock(L);
}

/* 删除指定索引上的元素
 * 
 * 备注：删除后，该位置之上的所有元素会下移一个槽位
 *       不能用伪索引来调用这个函数，因为其并不指向真实的栈上位置
 */
LUA_API void lua_remove (lua_State *L, int idx) {
  StkId p;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  while (++p < L->top) setobjs2s(L, p-1, p);
  L->top--;
  lua_unlock(L);
}

/* 把栈顶元素插入指定的有效索引处，并依次移动这个索引之上的元素
 *
 * 备注：不要用伪索引来调用这个函数，因为伪索引并不真正指向堆栈上的位置
 */
LUA_API void lua_insert (lua_State *L, int idx) {
  StkId p;
  StkId q;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  for (q = L->top; q>p; q--) setobjs2s(L, q, q-1);
  setobjs2s(L, p, L->top);
  lua_unlock(L);
}

/* 弹出栈顶的值，并用该值替换指定索引上的值
 *
 * 备注：因为是在指定索引上进行覆盖操作，所以不移动任何元素
 */
LUA_API void lua_replace (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  /* explicit test for incompatible code */
  if (idx == LUA_ENVIRONINDEX && L->ci == L->base_ci)
    luaG_runerror(L, "no calling environment");
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  if (idx == LUA_ENVIRONINDEX) {
    Closure *func = curr_func(L);
    api_check(L, ttistable(L->top - 1)); 
    func->c.env = hvalue(L->top - 1);
    luaC_barrier(L, func, L->top - 1);
  }
  else {
    setobj(L, o, L->top - 1);
    if (idx < LUA_GLOBALSINDEX)  /* function upvalue? */
      luaC_barrier(L, curr_func(L), L->top - 1);
  }
  L->top--;
  lua_unlock(L);
}

// 将栈上指定索引处的值作一个拷贝压入栈顶
LUA_API void lua_pushvalue (lua_State *L, int idx) {
  lua_lock(L);
  setobj2s(L, L->top, index2adr(L, idx));
  api_incr_top(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
以下这些函数都是用于操作面向C环境的栈
*/

// 获取栈中元素在lua中的类型
LUA_API int lua_type (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (o == luaO_nilobject) ? LUA_TNONE : ttype(o);
}


LUA_API const char *lua_typename (lua_State *L, int t) {
  UNUSED(L);
  return (t == LUA_TNONE) ? "no value" : luaT_typenames[t];
}

/*
 *  基础API提供了lua_is*系列函数用于检查栈中该索引处元素是否是一个指定的类型
 *
 */
// 检查该索引处的元素类型是否是 LUA_TFUNCTION 且是C闭包
LUA_API int lua_iscfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return iscfunction(o);
}

/* 检查该索引处的元素类型是LUA_TNUMBER
 *
 * 备注：LUA_TNUMBER 或者是可以转换成LUA_TNUMBER的字符串都会判断为true
 */
LUA_API int lua_isnumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  return tonumber(o, &n);
}

/* 检查该索引处的元素类型是LUA_TSTRING
 *
 * 备注：LUA_TSTRING或者是LUA_TNUMBER都会判断为true
 */
LUA_API int lua_isstring (lua_State *L, int idx) {
  int t = lua_type(L, idx);
  return (t == LUA_TSTRING || t == LUA_TNUMBER);
}


LUA_API int lua_isuserdata (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return (ttisuserdata(o) || ttislightuserdata(o));
}


LUA_API int lua_rawequal (lua_State *L, int index1, int index2) {
  StkId o1 = index2adr(L, index1);
  StkId o2 = index2adr(L, index2);
  return (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
         : luaO_rawequalObj(o1, o2);
}


LUA_API int lua_equal (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0 : equalobj(L, o1, o2);
  lua_unlock(L);
  return i;
}


LUA_API int lua_lessthan (lua_State *L, int index1, int index2) {
  StkId o1, o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2adr(L, index1);
  o2 = index2adr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
       : luaV_lessthan(L, o1, o2);
  lua_unlock(L);
  return i;
}

/*
 *  基础API 提供了lua_to*系列函数用于获取指定类型的栈中元素
 *
 *  注意：正索引通常用于获取栈底，负索引通常用于获取栈顶
 *        lua_to*系列函数不会对栈上元素执行弹出操作     
 */
// 把索引处的lua值转换为lua_Number类型的C类型
LUA_API lua_Number lua_tonumber (lua_State *L, int idx) {
  TValue n;

  // 根据索引号取出对应的值
  const TValue *o = index2adr(L, idx);

  // 该lua值必须是一个数字或是可转换成数字的字符串
  if (tonumber(o, &n))
    return nvalue(o);
  else
    return 0;
}

/* 把索引处的lua值转换为带符号的整数类型
 */
LUA_API lua_Integer lua_tointeger (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2adr(L, idx);
  if (tonumber(o, &n)) {
    lua_Integer res;
    lua_Number num = nvalue(o);
    lua_number2integer(res, num);
    return res;
  }
  else
    return 0;
}


LUA_API int lua_toboolean (lua_State *L, int idx) {
  const TValue *o = index2adr(L, idx);
  return !l_isfalse(o);
}

// 把索引处的lua值转换为一个C字符串
LUA_API const char *lua_tolstring (lua_State *L, int idx, size_t *len) {
  StkId o = index2adr(L, idx);
  if (!ttisstring(o)) {
    lua_lock(L);  /* `luaV_tostring' may create a new string */
    if (!luaV_tostring(L, o)) {  /* conversion failed? */
      if (len != NULL) *len = 0;
      lua_unlock(L);
      return NULL;
    }
    luaC_checkGC(L);
    o = index2adr(L, idx);  /* previous call may reallocate the stack */
    lua_unlock(L);
  }
  if (len != NULL) *len = tsvalue(o)->len;
  return svalue(o);
}

/* 获取索引处的lua对象的长度
 *
 * 备注：对于lua字符串和table，就是使用长度操作符"#"的结果
 */
LUA_API size_t lua_objlen (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TSTRING: return tsvalue(o)->len;
    case LUA_TUSERDATA: return uvalue(o)->len;
    case LUA_TTABLE: return luaH_getn(hvalue(o));
    case LUA_TNUMBER: {
      size_t l;
      lua_lock(L);  /* `luaV_tostring' may create a new string */
      l = (luaV_tostring(L, o) ? tsvalue(o)->len : 0);
      lua_unlock(L);
      return l;
    }
    default: return 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!iscfunction(o)) ? NULL : clvalue(o)->c.f;
}


LUA_API void *lua_touserdata (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TUSERDATA: return (rawuvalue(o) + 1);
    case LUA_TLIGHTUSERDATA: return pvalue(o);
    default: return NULL;
  }
}


LUA_API lua_State *lua_tothread (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  return (!ttisthread(o)) ? NULL : thvalue(o);
}

// 把给定索引处的lua值转换为一般的C指针
LUA_API const void *lua_topointer (lua_State *L, int idx) {
  StkId o = index2adr(L, idx);
  switch (ttype(o)) {
    case LUA_TTABLE: return hvalue(o);
    case LUA_TFUNCTION: return clvalue(o);
    case LUA_TTHREAD: return thvalue(o);
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
      return lua_touserdata(L, idx);
    default: return NULL;
  }
}



/*
 *  基础API提供了lua_push*系列函数用于将C value压栈以便传递给lua
*/

// 向lua栈中压入一个常量nil
LUA_API void lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue(L->top);
  api_incr_top(L);
  lua_unlock(L);
}

// 向lua栈中压入一个双精度浮点数
LUA_API void lua_pushnumber (lua_State *L, lua_Number n) {
  lua_lock(L);
  setnvalue(L->top, n);
  api_incr_top(L);
  lua_unlock(L);
}

// 向lua栈中压入一个整型值
LUA_API void lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setnvalue(L->top, cast_num(n));
  api_incr_top(L);
  lua_unlock(L);
}

/* 向lua栈中压入一个任意字符串
 *
 * @s   - 指向一串字符
 * @len - 显式指定字符串长度
 *
 * 备注： 本API支持不带"\0"结尾的字符串
 */
LUA_API void lua_pushlstring (lua_State *L, const char *s, size_t len) {
  lua_lock(L);
  luaC_checkGC(L);
  setsvalue2s(L, L->top, luaS_newlstr(L, s, len));
  api_incr_top(L);
  lua_unlock(L);
}

/* 向lua栈中压入一个任意字符串
 *
 * @s   - 指向一个"\0"结尾的字符串
 */
LUA_API void lua_pushstring (lua_State *L, const char *s) {
  if (s == NULL)
    lua_pushnil(L);
  else
    lua_pushlstring(L, s, strlen(s));
}


LUA_API const char *lua_pushvfstring (lua_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  lua_lock(L);
  luaC_checkGC(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  lua_unlock(L);
  return ret;
}


LUA_API const char *lua_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  lua_lock(L);
  luaC_checkGC(L);
  va_start(argp, fmt);
  ret = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_unlock(L);
  return ret;
}

// 将C函数压栈的实质是将对应的C闭包压栈
// 创建C闭包，并把这个C闭包压栈
LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) {
  Closure *cl;
  lua_lock(L);
  luaC_checkGC(L);
  api_checknelems(L, n);

  // 创建一个C闭包,继承创建了这个闭包的环境
  cl = luaF_newCclosure(L, n, getcurrenv(L));

  // 把这个C函数注册到这个闭包里
  cl->c.f = fn;
  L->top -= n;

  // 如果这个C函数关联值个数n非0,将这些值依次拷贝到这个闭包里
  while (n--)
    setobj2n(L, &cl->c.upvalue[n], L->top+n);

  // 压入这个闭包到栈顶
  setclvalue(L, L->top, cl);
  lua_assert(iswhite(obj2gco(cl)));
  api_incr_top(L);
  lua_unlock(L);
}

// 将b作为一个boolean值压栈
LUA_API void lua_pushboolean (lua_State *L, int b) {
  lua_lock(L);
  setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushlightuserdata (lua_State *L, void *p) {
  lua_lock(L);
  setpvalue(L->top, p);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API int lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, L->top, L);
  api_incr_top(L);
  lua_unlock(L);
  return (G(L)->mainthread == L);
}



/*
** get functions (Lua -> stack)
*/

/* 顾名思义，本函数用于获取table中指定元素的值，类似"t[k]"，本函数可能会触发__index元方法
 *
 * 备注：t是指定索引idx处的值，k是栈顶(-1)处的值
 *       本函数会弹出栈顶的k，然后将获得的值"t[k]"压入栈顶
 */
LUA_API void lua_gettable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  luaV_gettable(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
}

/* 类似lua_gettable，区别在于"k"不来自栈顶而来自入参
 *
 * @idx - 该索引处存放了"t[k]"的值t
 * @k   - 传入了"t[k]"的值k
 */
LUA_API void lua_getfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  setsvalue(L, &key, luaS_new(L, k));
  luaV_gettable(L, t, &key, L->top);
  api_incr_top(L);
  lua_unlock(L);
}

/* 类似lua_gettable，区别在于本函数不会触发__index元方法
 *
 * 备注：t是指定索引idx处的值，k是栈顶(-1)处的值
 */
LUA_API void lua_rawget (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2s(L, L->top - 1, luaH_get(hvalue(t), L->top - 1));
  lua_unlock(L);
}

/* 类似lua_rawget，区别在于本函数只专门用于操作数组
 *
 * 备注：t是指定索引idx处的值，n是元素在数组中的索引
 */
LUA_API void lua_rawgeti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2s(L, L->top, luaH_getnum(hvalue(o), n));
  api_incr_top(L);
  lua_unlock(L);
}

/* 创建一个新的空table并压入栈
 *
 * 备注：这个新table将被预分配narray个元素的数组空间以及nrec个元素的非数组空间
 */
LUA_API void lua_createtable (lua_State *L, int narray, int nrec) {
  lua_lock(L);
  luaC_checkGC(L);
  sethvalue(L, L->top, luaH_new(L, narray, nrec));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API int lua_getmetatable (lua_State *L, int objindex) {
  const TValue *obj;
  Table *mt = NULL;
  int res;
  lua_lock(L);
  obj = index2adr(L, objindex);
  switch (ttype(obj)) {
    case LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      break;
    default:
      mt = G(L)->mt[ttype(obj)];
      break;
  }
  if (mt == NULL)
    res = 0;
  else {
    sethvalue(L, L->top, mt);
    api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  return res;
}


LUA_API void lua_getfenv (lua_State *L, int idx) {
  StkId o;
  lua_lock(L);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  switch (ttype(o)) {
    case LUA_TFUNCTION:
      sethvalue(L, L->top, clvalue(o)->c.env);
      break;
    case LUA_TUSERDATA:
      sethvalue(L, L->top, uvalue(o)->env);
      break;
    case LUA_TTHREAD:
      setobj2s(L, L->top,  gt(thvalue(o)));
      break;
    default:
      setnilvalue(L->top);
      break;
  }
  api_incr_top(L);
  lua_unlock(L);
}
/*
** access functions (stack -> C)
以上这些函数都是用于操作面向C的栈
*/


/*
** set functions (stack -> Lua)
*/

/* 顾名思义，本函数做了一个等价于"t[k] = v"的操作，本函数可能会触发__newindex元方法
 *
 * 备注：t是指定索引idx处的值，v是栈顶(-1)处的值，k是栈顶之下(-2)那个值
 *       本函数运行中会弹出栈中的v和k值
 */
LUA_API void lua_settable (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  luaV_settable(L, t, L->top - 2, L->top - 1);
  L->top -= 2;  /* pop index and value */
  lua_unlock(L);
}

/* 类似lua_settable，做了一个等价于"t[k] = v"的操作，区别在于k来自入参
 *
 * @k   - 传入了"t[k]"的值k
 *
 * 备注：t是指定索引idx处的值，v是栈顶(-1)处的值
 *       本函数运行中会弹出栈中的v值
 */
LUA_API void lua_setfield (lua_State *L, int idx, const char *k) {
  StkId t;
  TValue key;
  lua_lock(L);
  api_checknelems(L, 1);
  t = index2adr(L, idx);
  api_checkvalidindex(L, t);
  setsvalue(L, &key, luaS_new(L, k));

  // 从栈顶弹出一个值赋给t[key]
  luaV_settable(L, t, &key, L->top - 1);
  L->top--;  /* pop value */
  lua_unlock(L);
}

/* 类似lua_settable，做了一个等价于"t[k] = v"的操作，区别在于本函数不会触发__newindex元方法
 *
 * 备注：t是指定索引idx处的值，v是栈顶(-1)处的值，k是栈顶之下(-2)那个值
 */
LUA_API void lua_rawset (lua_State *L, int idx) {
  StkId t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  setobj2t(L, luaH_set(L, hvalue(t), L->top-2), L->top-1);
  luaC_barriert(L, hvalue(t), L->top-1);
  L->top -= 2;
  lua_unlock(L);
}

/* 类似lua_rawset，做了一个等价于"t[n] = v"的操作，区别在于本函数只专门用于操作数组
 *
 * 备注：t是指定索引idx处的值，n是元素在数组中的索引，v是栈顶(-1)处的值
 */
LUA_API void lua_rawseti (lua_State *L, int idx, int n) {
  StkId o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_check(L, ttistable(o));
  setobj2t(L, luaH_setnum(L, hvalue(o), n), L->top-1);
  luaC_barriert(L, hvalue(o), L->top-1);
  L->top--;
  lua_unlock(L);
}


LUA_API int lua_setmetatable (lua_State *L, int objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checknelems(L, 1);
  obj = index2adr(L, objindex);
  api_checkvalidindex(L, obj);
  if (ttisnil(L->top - 1))
    mt = NULL;
  else {
    api_check(L, ttistable(L->top - 1));
    mt = hvalue(L->top - 1);
  }
  switch (ttype(obj)) {
    case LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarriert(L, hvalue(obj), mt);
      break;
    }
    case LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      if (mt)
        luaC_objbarrier(L, rawuvalue(obj), mt);
      break;
    }
    default: {
      G(L)->mt[ttype(obj)] = mt;
      break;
    }
  }
  L->top--;
  lua_unlock(L);
  return 1;
}


LUA_API int lua_setfenv (lua_State *L, int idx) {
  StkId o;
  int res = 1;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2adr(L, idx);
  api_checkvalidindex(L, o);
  api_check(L, ttistable(L->top - 1));
  switch (ttype(o)) {
    case LUA_TFUNCTION:
      clvalue(o)->c.env = hvalue(L->top - 1);
      break;
    case LUA_TUSERDATA:
      uvalue(o)->env = hvalue(L->top - 1);
      break;
    case LUA_TTHREAD:
      sethvalue(L, gt(thvalue(o)), hvalue(L->top - 1));
      break;
    default:
      res = 0;
      break;
  }
  if (res) luaC_objbarrier(L, gcvalue(o), hvalue(L->top - 1));
  L->top--;
  lua_unlock(L);
  return res;
}


/*
** `load' and `call' functions (run Lua code)
*/


#define adjustresults(L,nres) \
    { if (nres == LUA_MULTRET && L->top >= L->ci->top) L->ci->top = L->top; }


#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)))
	
/* 调用一个lua函数
 * @nargs   - 压入栈中的参数个数
 * @nresults- 返回值的个数
 *
 * 备注：要调用一个lua函数前需要确保已经执行了以下操作：
 *      [1]. 要调用的lua函数应该首先被压入栈
 *      [2]. 然后把该函数的入参正序压入栈(也就是第一个参数首先压栈)
 *      [3]. 最后调用一下lua_call
 *
 *       当lua_call返回时，所有的入参以及要调用的lua函数都会出栈，而函数的返回值则同样按照正序被压入栈
 */
LUA_API void lua_call (lua_State *L, int nargs, int nresults) {
  StkId func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  func = L->top - (nargs+1);
  luaD_call(L, func, nresults);
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to `f_call' */
  StkId func;
  int nresults;
};


static void f_call (lua_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_call(L, c->func, c->nresults);
}



LUA_API int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc) {
  struct CallS c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else {
    StkId o = index2adr(L, errfunc);
    api_checkvalidindex(L, o);
    func = savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  c.nresults = nresults;
  status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}


/*
** Execute a protected C call.
*/
struct CCallS {  /* data to `f_Ccall' */
  lua_CFunction func;
  void *ud;
};


static void f_Ccall (lua_State *L, void *ud) {
  struct CCallS *c = cast(struct CCallS *, ud);
  Closure *cl;
  cl = luaF_newCclosure(L, 0, getcurrenv(L));
  cl->c.f = c->func;
  setclvalue(L, L->top, cl);  /* push function */
  api_incr_top(L);
  setpvalue(L->top, c->ud);  /* push only argument */
  api_incr_top(L);
  luaD_call(L, L->top - 2, 0);
}


LUA_API int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) {
  struct CCallS c;
  int status;
  lua_lock(L);
  c.func = func;
  c.ud = ud;
  status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0);
  lua_unlock(L);
  return status;
}


LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname) {
  ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname);
  lua_unlock(L);
  return status;
}


LUA_API int lua_dump (lua_State *L, lua_Writer writer, void *data) {
  int status;
  TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = L->top - 1;
  if (isLfunction(o))
    status = luaU_dump(L, clvalue(o)->l.p, writer, data, 0);
  else
    status = 1;
  lua_unlock(L);
  return status;
}


LUA_API int  lua_status (lua_State *L) {
  return L->status;
}


/*
** Garbage-collection function
*/

LUA_API int lua_gc (lua_State *L, int what, int data) {
  int res = 0;
  global_State *g;
  lua_lock(L);
  g = G(L);
  switch (what) {
    case LUA_GCSTOP: {
      g->GCthreshold = MAX_LUMEM;
      break;
    }
    case LUA_GCRESTART: {
      g->GCthreshold = g->totalbytes;
      break;
    }
    case LUA_GCCOLLECT: {
      luaC_fullgc(L);
      break;
    }
    case LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(g->totalbytes >> 10);
      break;
    }
    case LUA_GCCOUNTB: {
      res = cast_int(g->totalbytes & 0x3ff);
      break;
    }
    case LUA_GCSTEP: {
      lu_mem a = (cast(lu_mem, data) << 10);
      if (a <= g->totalbytes)
        g->GCthreshold = g->totalbytes - a;
      else
        g->GCthreshold = 0;
      while (g->GCthreshold <= g->totalbytes) {
        luaC_step(L);
        if (g->gcstate == GCSpause) {  /* end of cycle? */
          res = 1;  /* signal it */
          break;
        }
      }
      break;
    }
    case LUA_GCSETPAUSE: {
      res = g->gcpause;
      g->gcpause = data;
      break;
    }
    case LUA_GCSETSTEPMUL: {
      res = g->gcstepmul;
      g->gcstepmul = data;
      break;
    }
    default: res = -1;  /* invalid option */
  }
  lua_unlock(L);
  return res;
}



/*
** miscellaneous functions
*/


LUA_API int lua_error (lua_State *L) {
  lua_lock(L);
  api_checknelems(L, 1);
  luaG_errormsg(L);
  lua_unlock(L);
  return 0;  /* to avoid warnings */
}


LUA_API int lua_next (lua_State *L, int idx) {
  StkId t;
  int more;
  lua_lock(L);
  t = index2adr(L, idx);
  api_check(L, ttistable(t));
  more = luaH_next(L, hvalue(t), L->top - 1);
  if (more) {
    api_incr_top(L);
  }
  else  /* no more elements */
    L->top -= 1;  /* remove key */
  lua_unlock(L);
  return more;
}


LUA_API void lua_concat (lua_State *L, int n) {
  lua_lock(L);
  api_checknelems(L, n);
  if (n >= 2) {
    luaC_checkGC(L);
    luaV_concat(L, n, cast_int(L->top - L->base) - 1);
    L->top -= (n-1);
  }
  else if (n == 0) {  /* push empty string */
    setsvalue2s(L, L->top, luaS_newlstr(L, "", 0));
    api_incr_top(L);
  }
  /* else n == 1; nothing to do */
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  lua_lock(L);
  if (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  return f;
}


LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


LUA_API void *lua_newuserdata (lua_State *L, size_t size) {
  Udata *u;
  lua_lock(L);
  luaC_checkGC(L);
  u = luaS_newudata(L, size, getcurrenv(L));
  setuvalue(L, L->top, u);
  api_incr_top(L);
  lua_unlock(L);
  return u + 1;
}




static const char *aux_upvalue (StkId fi, int n, TValue **val) {
  Closure *f;
  if (!ttisfunction(fi)) return NULL;
  f = clvalue(fi);
  if (f->c.isC) {
    if (!(1 <= n && n <= f->c.nupvalues)) return NULL;
    *val = &f->c.upvalue[n-1];
    return "";
  }
  else {
    Proto *p = f->l.p;
    if (!(1 <= n && n <= p->sizeupvalues)) return NULL;
    *val = f->l.upvals[n-1]->v;
    return getstr(p->upvalues[n-1]);
  }
}


LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  lua_lock(L);
  name = aux_upvalue(index2adr(L, funcindex), n, &val);
  if (name) {
    setobj2s(L, L->top, val);
    api_incr_top(L);
  }
  lua_unlock(L);
  return name;
}


LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  StkId fi;
  lua_lock(L);
  fi = index2adr(L, funcindex);
  api_checknelems(L, 1);
  name = aux_upvalue(fi, n, &val);
  if (name) {
    L->top--;
    setobj(L, val, L->top);
    luaC_barrier(L, clvalue(fi), L->top);
  }
  lua_unlock(L);
  return name;
}

