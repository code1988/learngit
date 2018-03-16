元表，顾名思义，它本身就是一个table，再具体一点说，就是用来存放元方法的table。
元表的作用就是修改一个值的行为(更确切的说，这是元方法的能力)，需要注意的是，这种修改会覆盖掉原本该值可能存在的相应的预定义行为。

1. lua中的每个值都可以有一个元表，只是table和userdata可以有各自独立的元表，而其他类型的值则共享其类型所属的单一元表。
   lua代码中只能设置table的元表，至于其他类型值的元表只能通过C代码设置。
   多个table可以共享一个通用的元表，但是每个table只能拥有一个元表。

2. 算术类和关系类元方法主要有以下几种：
        __add   - 加法
        __sub   - 减法
        __mul   - 乘法
        __div   - 除法
        __unm   - 相反数
        __mod   - 取模
        __pow   - 乘幂
        __concat    - 描述连接操作符".."行为
        __eq    - 等于
        __le    - 小于等于
        __lt    - 小于
   对于二元操作符，按照从左到右的顺序从操作对象中查找对应的元方法。

3. 库定义的元方法主要有：
        __tostring  - 用于重定义各种类型的值的print输出。
                      调用print输出某个值时会检查该值是否有一个__tostring的元方法，如果有，就用这个元方法来生成输出字符串
        __metatable - 用于保护集合的元表，使用户既不能看也不能修改集合的元表
                      当一个集合设置定义了该字段的元表后，后续调用getmetatable就会返回该字段的值，而调用setmetatable则会引发一个错误

4. __index元方法用于改变访问table中不存在的key时的行为。
   当访问一个table中不存在的key时，如果在该table的元表中定义了__index字段，那么lua就会用调用该元方法，规则如下：
            当__index元方法是一个函数时，lua会用table和不存在的key作为入参来调用该函数;
            当__index元方法是一个table时，lua就会用不存在的key访问这个table。

   如果不想在访问一个table时自动调用到它的__index元方法，就需要使用函数rawget。

   一个典型的__index元方法应用案例就是实现具有默认值的table:

        function setDefault (t,d)
            local mt = {__index = function (t,k) return d end}
            setmetatable(t,mt)
            return t
        end
        tb = {1,2}
        setDefault(tb,6)
        print(tb[1],tb[10])    --> 1,6
        --[[
        以上代码中，调用了setDefault后，任何对tb中不存在字段的访问都将调用它的__index元方法，
        而这个元方法总是返回d的值(这里就是6)。

        但是用这种方式实现具有默认值的table存在一个缺陷：需要为每个具有默认值的table都创建一个对应的元表.
        --]]
              
        local mt = {__index = function (t,k) return t.___ end}
        function setDefault (t,d)
            t.___ = d
            setmetatable(t,mt)
        end
        tb = {1,2}
        setDefault(tb,6)
        print(tb[1],tb[10])    --> 1,6
        --[[
        以上代码中，独立出来的元表mt可以用于所有table，因为默认值被存储在每个table中的固定字段"___"，而不再跟元表关联.
        
        但是这种方法需要注意命名冲突问题.
        --]]
        
        local key = {}
        local mt = {__index = function (t,k) return t[key] end}
        function setDefault (t,d)
            t[key] = d
            setmetatable(t,mt)
        end
        tb = {1,2}
        setDefault(tb,6)
        print(tb[1],tb[10])    --> 1,6
        --[[
        以上代码中，创建了一个独立的table，并用它的地址作为key，从而确保这个key在所有table中的唯一性
        --]]

5. __newindex元方法用于改变设置table中不存在的key时的行为。
   当对一个table中不存在的key赋值时，如果在该table的元表中定义了__newindex字段，那么lua就会调用该元方法，规则基本类似__index
   
   如果不想在设置一个table时自动调用到它的__newindex元方法，就需要使用函数rawset。

6. __index和__newindex的应用案例： 监视对一个table的所有访问情况
   实现原理：为真正的table创建一个代理，这个代理就是一个空的table(只有将一个table永远保持为空，才能捕捉到所有对它的访问)， 
             并通过__index和__newindex将访问重定向到原来的table上。
                
        tb = {1,2}    -- 构造一个需要被监控的table
        local _tb = tb      -- 保持对原table的一个私有访问
        tb = {}             -- 创建全局代理,实际就是将持有原table的全局变量重定义为一个空table
        
        local mt = {
            __index = 
            function (t,k)
                print("access to element " .. tostring(k))
                return _tb[k]
            end,
            __newindwx = 
            function (t,k,v)
                print("update of element ".. tostring(k).. " to " .. tostring(v))
                _tb[k] = v
            end
        }

        setmetatable(tb,mt) -- 设置代理table的元表
        
        print(tb[1])    -- access to element 1
                        -- 1
        tb[3] = 3       -- update of element 3 to 3
        --[[
        以上代码中，原table被保存在一个私有变量_tb中，外界只能通过代理table来访问原table，而整个访问情况通过代理table的元表被记录下来.

        当然这种代理方法存在2个缺点:
            [1]. 无法遍历原来的table，因为pairs只能操作代理table
            [2]. 如果要同时监视多个table，就需要为每个table创建对应的元表，解决方法类似上面的"实现具有默认值的table",即将原来的table保存在代理table的一个特殊字段中
        --]]

7. __index和__newindex的应用案例：实现只读的table
   实现原理：同样是为真正的table创建一个空的代理table，不同的是只需要监控所有对table的写操作，并引发一个错误就行。
             至于读操作，直接使用原table作为__index元方法即可。

        tb = {1,2}    -- 构造一个需要被监控的table
        local _tb = tb      -- 保持对原table的一个私有访问
        tb = {}             -- 创建全局代理,实际就是将持有原table的全局变量重定义为一个空table
        
        local mt = {
            __index = _tb,
            __newindex = 
            function (t,k,v)
                error("attempt to update a read-only table",2)
            end
        }

        setmetatable(tb,mt) -- 设置代理table的元表
        
        print(tb[1])    -- 1
        tb[1] = 2       -- error: attempt to update a read-only table


