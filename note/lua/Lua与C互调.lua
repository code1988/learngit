                    Lua作为一个库嵌入到C语言中使用的情况
--------------------------------------------------------------------------------------------                                    
一个典型的例子就是lua解释器。
C程序中调用lua代码块的标准套路如下：
        #include <lua.h>
        #include <lualib.h>
        #include <lauxlib.h>

        int main()
        {
            lua_State *L = luaL_newstate();
            luaL_openlibs(L);

            if(luaL_loadfile(L,"./test.lua") || lua_pcall(L,0,0,0))
                printf("error:%s\n",lua_tostring(L,-1));

            lua_getglobal(L,"mytest");

            if(lua_pcall(L,0,0,0))
                printf("error:%s\n",lua_tostring(L,-1));

            return 0;
        }

1. C语言中调用lua代码块的第一步总是创建一个lua虚拟机.
   相关API：
            luaL_newstate   -- 新创建的lua虚拟机中没有包含任何预定义的函数，甚至没有print，这么做的目的是尽可能保持语言本身的小巧
            #define lua_open()  luaL_newstate() -- 显然，这个宏实际就是luaL_newstate

2. 开启lua标准库属于可选操作
   相关API：
            luaL_openlibs   -- 打开所有的标准库，这些标准库也可以根据需求单独开启

3. 加载lua代码块
   相关API:
            luaL_loadfile(filename)         -- 加载程序文件中的lua代码块，文件名filename需要带路径名
            luaL_loadbuffer(buff,size,name) -- 加载缓冲中的lua代码块，name就是代码块(匿名函数)名称
            luaL_loadstring(s)              -- 加载字符串中的lua代码块，其实就是简单封装了luaL_loadbuffer
            
4. 运行lua代码块
   相关API:
            lua_call(nargs,nresults)        --[[ 调用一个lua代码块(由于lua 把一个 chunk 当作一个拥有不定参数的匿名函数，所以本函数也可理解为调用一个lua函数)
                                                 @nargs   - 压入栈中的参数个数 
                                                 @nresults- 返回值的个数
                                                 备注：要调用一个lua函数前需要确保已经执行了以下操作：
                                                        [1]. 要调用的lua函数应该首先被压入栈
                                                        [2]. 然后把该函数的入参正序压入栈(也就是第一个参数首先压栈)
                                                        [3]. 最后调用一下lua_call
                                                 lua_call返回时，所有的入参以及要调用的lua函数都会出栈，而函数的返回值则同样按照正序被压入栈--]]
            lua_pcall(nargs,nresults,errfunc)   --[[ 以保护模式调用一个lua代码块
                                                     @nargs   - 压入栈中的参数个数
                                                     @nresults- 返回值的个数
                                                     @errfunc - 错误处理函数在栈上的索引，如果存在错误处理函数(errfunc非0)，则必须先将其压入栈中，也就是必须位于待调用函数及其参数的下面
                                                     备注：如果在调用过程中没有发生错误，或者errfunc=0， 则lua_pcall 的行为和 lua_call 完全一致;
                                                           否则在发生运行错误时的行为顺序如下：
                                                            [1]. 调用errfunc索引处的错误处理函数，入参就是原始错误信息
                                                            [2]. 错误处理函数的返回值将作为出错信息压栈
                                                            [3]. 返回调用函数本身的返回值，也就是错误代码LUA_ERR*
                                                     通常用法是，错误处理函数被用来在出错信息上加上更多的调试信息，以获得更详细的出错原因--]]
            lua_cpcall(func,ud)      -- 以保护模式调用一个c函数func,ud是它的唯一入参
--------------------------------------------------------------------------------------------                                    


                    C作为一个库嵌入到Lua中使用的情况
--------------------------------------------------------------------------------------------                                    
所有注册到lua中的C函数必须遵循的格式: 
                typedef int (lua_CFunction *)(lua_Stat *L)  -- C函数从栈中获取参数，并将结果压入栈中，同时返回一个整数，表示其压入栈中的返回值数量。

典型的例子就是lua的标准库.
实现一个C库的标准套路如下：
        static int do_init(lua_State *L)
        {
            printf("hello world\n");
            return 0;
        }

        static const luaL_Reg mylib[] = {
            {"init", do_init},
            {NULL,NULL}
        };

        int luaopen_mylib(lua_State *L)
        {
            luaL_register(L,"mylib",mylib);
            return 1;
        }

1. C库必须定义一个特殊的函数luaopen_*，这个函数用于注册模块中所有的C函数，以及一些本模块相关的初始化.
   通常，C模块中中只有这一个对外函数，其他所有函数都是static的.
   
2. 模块中所有的C函数(除了luaopen_*)通常都会被收集到一个数组中，数组元素的类型为luaL_Reg结构：
        typedef struct luaL_Reg{
            const char *name;   // 在lua中的函数名
            lua_CFunction func; // C函数地址
        }luaL_Reg; 
   该数组的最后一个元素总是{NULL,NULL}，用以标识结尾

3. 实际注册C模块的API为：
    luaL_register(L,libname,l)
    @libname    - 模块名/库名
                  NULL时表示将表l中的所有C函数注册到栈顶的值中;
                  非NULL时表示将首先创建一个名为libname的全局table，然后将表l中的所有C函数注册到该table中.
    @l          - 需要注册的C函数集合
    备注：由于通过这种方式注册的C函数会成为lua全局环境中的变量，这种污染全局环境的方式并不合理，5.2开始的版本中被废除，取而代之的是luaL_setfuncs

4. 将C模块编译成动态链接库后，需要放入LUA_CPATH指定的C路径中，这样就可以在lua中加载该模块：
        require "mylib"
   
