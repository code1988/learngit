                                   lua调用C
/* ***************************************************************************************
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

    lua_Number lua_tonumber(lua_State *L,int index)
    把索引处的lua值转换为lua_Number类型(double)的C类型
*****************************************************************************************/
