1. lua解释器： #!/usr/local/bin/lua
   lua脚本首行带这行代码后，命令行就可以直接运行脚本

2. lua的关键词：
        and break do else elseif end false for function if in local nil not
        or repeat return then true until while 

3. lua的变量有三种：全局变量、局部变量、表中的域
   lua中的变量默认都是全局变量，除非用local声明(函数入参等是局部变量)
   lua可以对多个变量同时赋值，变量列表和值列表的各个元素用逗号分开
   lua是首先计算右边所有的值，然后再执行赋值操作，所以 x,y = y,x 可以实现交换变量值
   当变量个数和值个数不一致时，lua以变量个数为基础，采取少补nil，多忽略的原则

4. lua有8个基本数据类型：
        nil     - 只有1个值nil，表示无效值（条件表达式中相当于false）
                  对于全局变量和table，nil还有删除作用
        boolean - 包含2个值false、true
        number  - 双精度类型的实浮点数
                  lua会根据需要自动在number和string之间自动转换：
                  当对一个字符串进行算术操作时，string就会被转换成number（比如 print("1" + 2)）;
                  当期望一个string却碰到number时，number就会被转换成string（比如print(1 .. 2)）
        string  - 字符串，由一对“”或‘’来，也可以用"[[]]"来表示一块字符串
                  对一个数字字符串进行算术操作时，首先会将这个字符串转成数字
        function- 由C或lua编写的函数
        userdata- 任意存储在变量中的C数据结构
        thread  - lua里，最主要的线程是协同程序coroutine
                  协程跟线程共同点：拥有自己独立的栈、局部变量和指令指针，跟其他线程共享全局变量等
                  区别：线程可以同时运行多个，而协程任意时刻只能运行一个，处于运行状态的协程只有被挂起才会暂停
        table   - 表，也可以理解为数组，表的索引可以是数字或字符串,格式是"table[索引]"或"table.索引"，要注意到是，索引是数字时不支持第2种格式
                  需要注意a.x和a[x]的区别，a.x实际就是a["x"]，表示以字符串"x"来索引table，而a[x]表示以变量x的值来索引table
                  不同于C语言把0作为数组的初始索引，lua里表的默认初始索引是1
                  表不会固定长度大小，而是随着数据添加自动增长

5. lua提供3种循环处理方式：
    while循环 - 语法格式 -  while(条件) do 执行体   end
              - 备注     -  类C,略
    for循环又细分为：数值for循环 - 语法格式 -   for var=exp1,exp2,exp3 do 执行体    end
                                 - 备注     -   var从exp1变化到exp2,间隔exp3（可选，缺省值1）
                                                for的3个exp在循环开始前一次性求值，以后不再进行求值
                     泛型for循环 - 语法格式 -   for i,v in ipairs(a) do print(v)    end
                                 - 备注     -   i是table索引值，v是对应索引值的table元素值，ipairs是lua提供的一个迭代函数，用来迭代table
    repeat...until循环 - 语法格式 - repeat  执行体  while(条件)
                       - 备注     - 类C，略
    
6. lua流程控制语句 if...then...end
   lua中0为true，这点跟C不同，if语句要注意then这个关键词不能丢

7. lua中函数默认为全局函数，头部加local表示为局部函数
   lua函数可以返回多个值，以逗号隔开
   lua函数定义是可以匿名(只有关键字function标识而没有函数名)，可以定义在参数位置，作为参数传递
   lua函数可以接受可变形参，格式类似C语言使用"..."，参数保存在全局arg表或{...}中
   lua函数只有单个实参时，并且实参类型是字符串或table构造式（就是一个带"{xx,xxx}"的table定义式），函数调用中的圆括号可以省略

8. lua的算术运算符和关系运算符基本类C，唯一区别：~= 表示不等于
   lua的逻辑运算符：    and - 逻辑与操作符
                        or  - 逻辑或操作符
                        not - 逻辑非操作符
   其他运算符：         ..  - 连接两个字符串
                        #   - 返回字符串或表的长度,需要注意的是,使用本操作符取表的长度时,只有确保键值是连续的连续的数值键值(且不存在nil值)时才有意义

