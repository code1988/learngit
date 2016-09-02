1. lua的模块类似于一个封装库，以API接口的形式在其他地方被调用，模块的结构就是一个table的结构
    例：
        -- 模块文件的文件名 test.lua
        local M = {}        
        _G[...] = M         
        function M.fun()
            print("hello")
        end
        return M
        备注：  ...就是传递给模块的模块名，这里就是"test"
            在全局环境_G这个table里，添加一个以"test"作为索引的元素，内容是M
            于是当我们直接调用模块名test时，其实就是在调用_G["test"]的内容M
        问题：  实际编程中，return M语句的省略似乎并没有产生错误提示
        
        对于使用5.2版本以下的lua，模块文件还可以使用以下定义方法：
        module(...,package.seeall)
        function fun()
            print("hello")
        end
        备注：  module(...,package,seeall)函数相当于实现了
                    local M = {}
                    _G[...] = M
                    setmetatable(M,{__index = _G})
                    setfenv(1,M)
                    ...
                    return M
        问题：  module函数5.3之后被彻底抛弃，官方给出原因是会污染全局环境变量

2. lua通过一个require的函数来加载模块，require函数定义如下:
	require("模块名")
	备注：	因为模块有可能是lua库或c库，所以模块名不能包含扩展名;
			该函数会返回一个由模块函数组成的table

	require函数的加载原则：
		用模块名替换package.path中的每个“？”，然后检查是否存在这样一个文件;
		如果找不到，接着在package.cpath中找
