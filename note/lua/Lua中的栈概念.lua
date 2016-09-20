Lua中设计"栈"的目的就是解决Lua与C的通信。
无论是Lua到C，还是C到Lua，所有的数据交换都需要通过"栈"来完成，此外还可以用"栈"来保存一些中间结果。
"栈"解决了Lua和C语言之间存在的两大差异：
        [1]. lua自带垃圾回收机制，而C需要手动显式的释放内存
        [2]. lua使用动态类型，而C使用静态类型

"栈"实际是由lua管理的，"栈"中的每个元素都能保存任何类型的lua值，lua严格按照LIFO(后进先出)规则来操作栈，这些都意味着一个C值一旦入栈，就是进入了lua的世界。
不同于lua中只能操作"栈"的顶部，C-API则拥有更大的自由度，可以操作"栈"中任意位置的元素。

C-API是一组能使C代码与Lua交互的函数，这些API实现了读写Lua中的变量、调用Lua函数、运行一段Lua代码、注册C函数到Lua等功能。
几乎所有C-API实质都是对"栈"的操作，以下是对主要的C-API进行了分类。

1. 这部分API是纯粹对"栈"上元素的一些基本操作:
    -- lua_gettop()
    返回栈顶元素的正索引，也就是获取栈中元素的个数
    备注：返回0意味着栈为空

    -- lua_settop(idx)
    将栈顶设置到一个指定位置idx，即修改栈中元素的数量
    备注：idx为正时，如果比之前的栈顶高，会向栈中压入nil来补足大小，如果比之前的栈顶低，意味着多出来的元素会被丢弃;
          idx为负时，只有丢弃功能
        -- #define lua_pop(L,n)        lua_settop(L, -(n)-1)
        由lua_settop衍生出来的API宏，用于从栈中弹出n个元素
        备注：需要确保传入的n是个正数

    -- lua_pushvalue(idx)
    将栈上指定索引idx处的值作一个拷贝压入栈顶
    备注：本函数可以操作"伪索引"

    -- lua_remove(idx)
    删除指定的有效索引idx处的元素
    备注：删除后，该位置之上的所有元素会下移一个槽位;
          本函数不可以操作"伪索引"，因为其并不指向真实的栈上位置

    -- lua_insert(idx)
    弹出栈顶的值，插入到指定的有效索引idx处，并依次移动这个索引之上的元素
    备注：本函数不可以操作"伪索引"，因为伪索引并不真正指向堆栈上的位置

    -- lua_replace(idx)
    弹出栈顶的值，并用该值替换指定索引idx上的值
    备注：因为是在指定索引上进行覆盖操作，所以不移动任何元素;
          本函数可以操作"伪索引"

    -- lua_checkstack(size)
    检查栈中是否有size长度的空间
    备注：栈缺省的最小槽位数量是LUA_MINSTACK，一般情况下完全足够，
          但是当遇到需要占用大量栈空间的情况时，就需要调用本函数来检查栈中是否有足够的空间

2. lua_push*系列API用于将C类型的数据压入栈，使之转换为对应的lua类型的数据，以便进入lua的世界
    -- lua_pushnil
    往栈中压入一个常量nil
    -- lua_pushnumber
    往栈中压入一个双精度浮点数
    -- lua_pushinteger
    往栈中压入一个整数
    -- lua_pushboolean
    往栈中压入一个boolean值
    -- lua_pushlstring(s,len)
    往栈中压入一个长度为len的字符串s
    -- lua_pushstring(s)
    往栈中压入一个"\0"结尾的字符串s
    备注：lua中不会去持有指向外部(比如C中)字符串的指针，所以在通过栈往lua中传递字符串时，都会在lua中生成一份拷贝。
          基于这种机制，外部函数在操作完字符串压栈后立刻释放或修改该字符串，不会有任何问题。
    
    -- lua_pushfstring(fmt,...)
    往栈中压入一个格式化过的字符串fmt，并返回指向这个字符串的指针 
    备注：类似于C中的sprintf，但也有区别：
            [1]. 无需提供这个字符串的缓冲区，因为会由lua来管理这个缓存
            [2]. 本函数接受的格式化符号极为有限(仅支持%%、%s、%d、%f、%c)
    -- lua_pushvfstring(fmt,argp)
    基本类似于lua_pushfstring，区别仅仅在于可变形参的格式

    -- lua_pushcclosure(fn,n)
    将符合lua_CFunction格式的C函数压栈以创建一个C闭包，其中包含n个upvalue
    备注：创建C闭包的步骤：
            [1].将需要关联的任意数量upvalue依次压栈 
            [2].调用本函数将C函数fn改造成C闭包，并将这个C闭包压栈
    要注意的是，本函数调用过程中首先会从栈中弹出所有的upvalue来创建C闭包，然后才是将C闭包压栈
        -- #define lua_pushcfunction(L,f)  lua_pushcclosure(L, (f), 0) 
        由lua_pushcclosure衍生出来的API宏，用于将一个普通的C函数压栈
        备注：这个C函数必须按照lua_CFunction格式来定义;
              这个C函数没有upvalue(也就是意味着压入栈中的是一个普通的C函数)

    -- lua_pushlightuserdata
    往栈中压入一个C指针
    备注：在lua中light userdata是一个像数字一样的值（猜测是C指针指向的地址值）;

3. lua_is*系列API用于检查指定索引处的元素是否为特定的lua类型
