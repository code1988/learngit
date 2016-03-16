1. lua解释器： #!/usr/local/bin/lua
   lua脚本首行带这行代码后，命令行就可以直接运行脚本

2. lua的关键词：
        and break do else elseif end false for function if in local nil not
        or repeat return then true until while 

3. lua的变量不需要声明，变量不等于nil时，即表示存在,变量默认总是全局的
   lua的变量不需要定义类型，只需要赋值就能使用

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

