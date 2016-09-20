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
            luaL_loadfile(filename) -- 加载lua程序文件，文件名filename需要带路径名

            
   



                    C作为一个库嵌入到Lua中使用的情况
--------------------------------------------------------------------------------------------                                    
1. C函数必须遵循的格式       
    typedef int (lua_CFunction *)(lua_Stat *L)
                    
2. 相关的API函数
    void lua_register(lua_State *L,const char *name,lua_CFunction f)
    通过一个宏定义把C函数f注册到全局变量name中： 
        #define lua_register(L,n,f) (lua_pushcfuncton(L,f),lua_setglobal(L,n))
    具体的注册流程：
        创建C闭包，将C函数注册到该闭包  
        将C闭包压栈，此时位于栈顶  
        从栈顶弹出一个值，也就是C闭包,赋值给全局table中的元素name
        至此完成了C函数在lua全局table中的注册

    lua_to*(lua_State *L,int index)
    把索引处的"*"指定类型的lua值从栈中取出

    lua_push*(lua_State *L,C value)
    把一个"*"指定类型的C value压栈以便传递给lua
*****************************************************************************************/
