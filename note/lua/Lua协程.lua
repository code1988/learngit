协程,也叫协同式多线程,跟普通的多线程一样,一个协程就代表了一条独立的执行序列.
协程和普通多线程的区别在于:
    普通的多线程,也叫抢占式多线程,意味着线程间的调度通常是由操作系统来完成；
    而协程间的调度并不通过操作系统,而是由当前运行的协程显式的进行切换.

lua实现的协程是一种非对称式协程(asymmetric coroutine),这种协程机制需要通过两个函数来控制协程的执行,一个用于(重)调用协程,另一个用于挂起协程;
而一些其他语言中提供了一种对称式协程(symmetric coroutine),这种协程机制中只通过一个函数来切换协程的执行.

lua将所有关于协程的函数放在一个名为"coroutine"的table中，以下是主要的API介绍：

1. coroutine.create(f)
    创建一个新的协程，入参f是一个函数(该函数将作为协程的主函数)，返回一个thread类型的值，表示该协程对象

2. coroutine.status(co)
    返回协程co当前的状态。
    一个协程存在4种状态：
                当协程被创建时都处于"suspended"状态(也就是说协程不会再创建时自动执行，显然这点跟普通多线程不同)，除此之外，当协程主动挂起时也会处于"suspended"状态;
                当协程正在运行时，则处于"running"状态;
                当协程a唤醒协程b后，协程a就处于"normal"状态;
                当协程运行完主函数或因错误停止，则处于"dead"状态

3. coroutine.running()
    返回当前正在运行的协程对象,也就是一个thread类型的值.
    如果是在主线程中调用本函数则返回nil.

3. coroutine.resume(co [,...])
    启动或再次唤醒协程co执行.
    参数...是可选的，其含义有2种:
            首次启动协程co时，该协程会从主函数开始运行，而参数...会作为入参传递给主函数;
            再次唤醒协程co时，参数...会作为协程co中上次yield的返回值.
    从调用resume的主线程/协程的角度看，协程co在这期间的运行都发生在resume调用中，当协程co终止或挂起时，本次resume的调用才返回，返回值的情况分为3种：
            如果协程co正常终止，则返回true和主函数的返回值；
            如果协程co运行过程中调用yield挂起，则返回true和调用yield时传入的参数；
            如果协程co执行过程中发生错误，则返回false和错误消息。
    需要注意的是，resume是在保护模式下运行的，所以即便协程中发生任何错误都不会导致程序终止，而是将出错信息返回给调用者.

4. coroutine.yield(...)
    挂起当前正在运行的协程。
    参数...是可选的，这些参数都会作为resume的额外返回值。
    从调用yield的协程的角度看，所有在它挂起时发生的活动都发生在yield调用中，当唤醒该协程时，本次yield的调用才返回，返回值就是resume函数传入的参数。

5. coroutine.wrap(f)
    创建一个新的协程,入参f是一个函数(该函数将作为协程的主函数),返回一个类似resume的函数.
    wrap和create都可以用来创建协程,区别在于:
        [1]. wrap创建的协程,其句柄是隐藏的,意味着无法检查该协程的状态
        [2]. 调用wrap返回的函数时,它不是像resume工作在保护模式,这意味着协程执行时出错会导致程序终止
        [3]. 调用wrap返回的函数时,它在返回时没有像resume有第一个布尔量

6. 基于协程实现的带过滤功能"生产者-消费者"模型
    function send (data)
        coroutine.yield(data)
    end

    function recv (prod)
        local res,data = coroutine.resume(prod)
        return data
    end

    -- 创建一个生产者协程
    function producter ()
        return coroutine.create(function ()
            -- 循环从标准输入获取行数据
            while true do
            local data = io.read()
            send(data)
        end
        end)
    end

    -- 创建一个过滤器协程
    function filter (prod)
        return coroutine.create(function ()
            -- 循环接收生产者传递的数据,并添加行号,在转发给消费者
            for line = 1,math.huge do
            local data = recv(prod)
            data = string.format("%d %s",line,data)
            send(data)
        end
        end)
    end

    function consumer (ft)
        -- 消费者循环接收数据并打印到标准输出
        while true do
            local data = recv(ft)
            io.write(data,"\n")
        end
    end

    consumer(filter(producter()))
