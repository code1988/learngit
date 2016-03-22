1. lua解释器： #!/usr/local/bin/lua
   lua脚本首行带这行代码后，命令行就可以直接运行脚本

2. lua的关键词：
        and break do else elseif end false for function if in local nil not
        or repeat return then true until while 

3. lua的变量有三种：全局变量、局部变量、表中的域
   函数外的变量默认为全局变量，除非用local声明，函数内变量与函数参数默认为局部变量
   lua可以对多个变量同时赋值，变量列表和值列表的各个元素用逗号分开
   lua是首先计算右边所有的值，然后再执行赋值操作，所以 x,y = y,x 可以实现交换变量值
   当变量个数和值个数不一致时，lua以变量个数为基础，采取少补nil，多忽略的原则

4. lua有8个基本数据类型：
        nil     - 只有1个值nil，表示无效值（条件表达式中相当于false）
                  对于全局变量和table，nil还有删除作用
        boolean - 包含2个值false、true
        number  - 双精度类型的实浮点数
        string  - 字符串，由一对“”或‘’来，也可以用"[[]]"来表示一块字符串
                  对一个数字字符串进行算术操作时，首先会将这个字符串转成数字
                  ..用于字符串之间的连接
                  #用于计算字符串的长度
        function- 由C或lua编写的函数
        userdata- 任意存储在变量中的C数据结构
        thread  - lua里，最主要的线程是协同程序coroutine
                  协程跟线程共同点：拥有自己独立的栈、局部变量和指令指针，跟其他线程共享全局变量等
                  区别：线程可以同时运行多个，而协程任意时刻只能运行一个，处于运行状态的协程只有被挂起才会暂停
        table   - 表，也可以理解为数组，数组索引可以是数字或字符串
                  不同于C语言把0作为数组的初始索引，lua里表的默认初始索引是1
                  表不会固定长度大小，而是随着数据添加自动增长

5. lua提供3种循环处理方式：
    while循环 - 语法格式 -  while(条件) do 执行体   end
              - 备注     -  类C,略
    for循环又细分为：数值for循环 - 语法格式 -   for var=exp1,exp2,exp3 do 执行体    end
                                 - 备注     -   var从exp1变化到exp2,间隔exp3（可选，缺省值1）
                                                for的3个exp在循环开始前一次性求值，以后不再进行求值
                     泛型for循环 - 语法格式 -   for i,v in ipairs(a) do print(v)    end
                                 - 备注     -   i是数组索引值，v是对应索引值的数组元素值，ipais是lua提供的一个迭代函数，用来迭代数组
    repeat...until循环 - 语法格式 - repeat  执行体  while(条件)
                       - 备注     - 类C，略
    
6. lua流程控制语句 if...else
   lua中0为true，这点跟C不同，if语句要注意then这个关键词不能丢

7. lua中函数默认为全局函数，头部加local表示为局部函数
   lua函数可以返回多个值，以逗号隔开

