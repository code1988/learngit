1. chunk(代码块)，可以是一条语句，也可以是一系列语句的集合，还可以是函数。
   chunk拥有独立的"词法域"，意味着chunk内部声明的局部变量和局部函数，仅在该chunk内可见。
   chunk的定义方式主要有以下几种：
            [1]. do ... end 可以显式定义一个chunk的作用范围
            [2]. 在交互模式下，通常一行就是一个chunk
            [3]. 一个字符串
            [4]. 一个文件
   
2. 加载一个外部的lua代码块需要使用loadfile/loadstring函数，这两个函数主要是将lua代码块编译成一种中间形式，然后整体作为一个匿名函数返回。
   加载一个程序块仅仅相当于定义了一个函数体是整个代码块的匿名函数，并且该匿名函数具有可变长形参。
   需要注意的就是，由于在这个过程中代码块并不会被运行，所以代码块中的函数尚未被定义。

   dofile这个函数内部不但调用了loadfile，还进一步调用了创建的匿名函数，所以就相当于运行了整个外部lua代码块文件。
   需要注意到就是，在这个过程中代码块中的语句被执行，其中的每个函数才会作为一条赋值语句被定义，从此才可以被调用。

   比较dofile和loadfile，两者的使用差别主要是：
            [1]. dofile一次调用即可完成加载和运行，适合用在简单任务中;
            [2]. loadfile在出现错误时会返回错误值，可以自定义错误处理方法;
            [3]. 对于需要多次运行的文件，通过loadfile加载后，就可以反复调用它的匿名函数，实现一次编译多次使用。

3. 加载一个外部的C代码块通常使用动态链接机制，对应的函数就是package.loadlib，有一点类似于loadfile，那就是在这个过程中代码块并不会被运行。
   loadlib加载指定的动态库，并将一个指定的C函数作为匿名lua函数返回。

4. 基于上面的代码块概念，lua进一步设计了一套模块系统(显然模块是代码块的子集概念)。
   需要注意的是，尽管标准并没有限定模块必须要设计成一个table，并将所有需要导出的函数和变量放入其中，但是约定俗成的规则建议最好这么做。
   编写模块的方法如下：(模块文件的文件名 test.lua)
        例：
            test = {}
            function test.hello ()
                print("hello world")
            end
            return test
            --[[
            以上这个模块就是一个table，要注意的有几点：
                [1]. 一个标准的模块一定要有返回值(通常就是table)，否则require时就返回一个boolean值true而不是期望的table
                [2]. 持有这个table的变量名可以有多个，但必须确保至少有一个是模块名(这么做是为了确保require后可以通过模块名调用模块)
                [3]. 每个函数必须定义成table的成员，也就是函数名的前缀(包括"."在内的前半部分)必须是模块名
            --]]

            local M = {}
            test = M
            function M.hello ()
                print("hello world")
            end
            return M
            --[[
            以上这个模块使用局部变量M来定义模块内的函数，然后将这个局部变量赋予模块名。
            这么写的优点是只需要在整个模块中的一处写出模块名，方便了对模块的重命名。
            --]]

            local M = {}
            _G[...] = M
            function M.hello ()
                print("hello world")
            end
            return M
            --[[
            以上这个模块没有再定义一个名为模块名的全局变量，而是利用了require加载模块时会以模块名作为参数来调用模块内容(也就是代码块,也就是匿名函数)，
            所以模块中的"..."也就代表了模块名,进而将table直接赋值给全局_G表中的模块名字段。
            这么写的优点是完全避免了在模块文件中写模块名，如果要重命名，只需要修改模块的文件名即可。
            --]]

            local M = {}
            _G[...] = M
            package.loaded[...] = M
            function M.hello ()
                print("hello world")
            end
            --[[
            以上这个模块通过将模块table直接赋值给package.loaded，进一步消除了结尾的"return M"。
            这种消除return语句的方法是利用了require加载模块时最后会返回package.loaded[模块名]的值。
            消除return语句的意义是将所有与模块相关的设置任务集中在两模块开头，一来结构更清晰，二来防止了最后忘写。
            --]]
            
            local M = {}
            _G[...] = M
            package.loaded[...] = M
            setfenv(1,M)
            function hello ()
                print("hello world")
            end
            --[[
            对于使用5.2版本以下的lua，还可以使用以上定义模块的方法。
            以上这个模块通过调用setfenv(1,M)，将当前模块的全局环境设置为M(原本_G)，这样一来，接下去的函数定义都不需要再带前缀。
            这种修改当前全局环境的方法带来的缺点也很明显，那就是无法再访问原本_G环境中的全局变量了，所以执行上面的"print"时将会报错。
            解决原来全局环境中的变量无法访问的方法主要有3种:
                    [1]. 使用元表,当访问当前环境(这里是M)中不存在的变量时,就会去访问_G中访问该变量
                            setmatatable(M,{__index = _G})
                    [2]. 声明一个局部变量，用来保存原来的全局环境(这种方法的缺点就是每次调用原来环境的变量时都要加"_G."前缀)
                            local _G = _G
                    [3]. 将每个需要用到的原来环境的函数或模块都用局部变量定义一次
                            local print = print
            备注：以上3种方法要想生效，都要确保在setfenv(1,M)之前执行。
                  最合理最常用的还是第一种方法。
            问题：5.2版本之后去掉了setfenv系列函数，所以以上这种定义模块的方法不再适用
            --]]
            
            module(...,package.seeall)
            function hello ()
                print("hello world")
            end
            --[[
            对于使用5.2版本以下的lua，还可以使用以上定义模块的方法
            备注：  module(...,package,seeall)函数相当于实现了
                        local M = {}
                        _G[...] = M
                        package.loaded[...] = M
                        setmetatable(M,{__index = _G})
                        setfenv(1,M)
                        ...
            问题：  module函数5.2之后被彻底抛弃，官方给出原因是会污染全局环境变量
            --]]

   加载模块的方法相对唯一，那就是调用require，格式如下:
	        require("模块名")
   备注：因为模块有可能是lua库或c库，所以模块名不能包含扩展名;
		 标准的模块会返回一个由模块函数组成的table，以及持有该table的名为模块名的全局变量
   
   以下是require函数的具体实现原理：
            function require (name)
                -- 检查模块是否已经加载
                if not package.loaded[name] then
                    -- 对于没有加载的模块，这里按四种方式顺序尝试加载：
                    --      [1]. 在package.preload这个table中根据模块名查找对应的函数，如果存在就作为模块的加载器(通常不会在package.preload中找到指定模块的条目)
                    --      [2]. 尝试从lua文件中加载模块，搜索路径定义在package.path中，如果找到，就用loadfile来加载该lua文件
                    --      [3]. 尝试从C动态库中加载模块，搜索路径定义在package.cpath中，如果找到，就用package.loadlib来加载该C文件
                    --      [4]. 尝试使用"all-in-one"加载器，类似于加载加载C库，只是查找的"luaopen_*"函数名不一样
                    -- 需要注意的是，加载成功的代码块到这里还并不会被执行，而是被定义为一个函数，将作为加载器使用
                    local loader = findloader(name)
                    if loader == nil then
                        error("unable to load module ".. name)
                    end

                    -- 在调用加载器之前，先将该模块标记为已加载
                    package.loaded[name] = true
                    -- 以模块名作为参数调用加载器(即模块,即上面已经加载的代码块)
                    local res = loader(name)
                    -- 如果加载器有返回值(标准的模块返回值就是table)，这里就将其存入package.loaded中
                    -- 这里也就意味着如果没有返回值，package.loaded中的值还是先前的true
                    if res ~= nil then
                        package.loaded[name] = res
                    end
                end

                -- 对于已经加载的模块，require调用实际什么也不做就直接返回相应的值
                return package.loaded[name]
            end

