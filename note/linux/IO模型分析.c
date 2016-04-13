1. I/O发生时涉及的对象和步骤
    以network I/O为例，它会涉及到两个对象，一个是调用这个I/O的process或thread，另一个就是kernel
    以read操作为例，从kernel角度看，整个执行步骤分为2个阶段：
        [1]. 等待数据准备
        [2]. 将数据从kernel拷贝到用户进程中

2. Blocking I/O模型
    用户进程调用了read系统调用，kernel就进入第一阶段：等待数据准备
    而在用户进程这边，整个进程会被block 
    kernel一直等到数据准备好了，进入第二阶段：将数据从kernel拷贝到用户进程中，然后kernel返回结果
    用户进程收到结果才解除block状态，重新运行起来

    所以，blocking I/O模型的特点就是在I/O执行的两个阶段都被block了

3. non-blocking I/O 模型
   用户进程调用了read系统调用，kernel中如果数据还没有准备好，则立即返回一个error
   用户进程得到了一个结果（error），通过判断这个结果可以知道数据是否已经获取，如果尚未获取，则一直重复以上操作
   一旦kernel中数据准备好了，并且又再次收到了用户进程的系统调用read，那么就将数据从kernel拷贝到用户进程中，然后kernel返回结果

   所以，non-block I/O模型的实质就是：
    [1]. I/O执行的第一阶段是non-block的，用户进程其实需要不断的主动轮询kernel数据好了没有
    [2]. I/O执行的第二阶段还是block的，这个结论看起来比较隐含

4. I/O Multiplexing 模型（本模型最典型的应用就是select，这里以此为例）
    本模型的特点就是单个process或thread就可以处理多个I/O

    用户进程调用了select后，整个进程会被block，同时，kernel会监视所有select负责的socket
    当任何一个socket中的数据准备好了，select就会返回
    这时候用户进程再调用read系统调用，将数据从kernel拷贝到用户进程

    所以，I/O Multiplexing 中的select模型和Blocking I/O模型其实很像，在I/O执行的的两个阶段都被block了
    两者的区别在于：
        [1]. select可以同时管理多个I/O;
        [2]. select在I/O执行的两个阶段之间要醒来一次（从kernel切换到用户进程，再切换回kernel），而Blocking I/O模型不需要
    I/O Multiplexing模型的优势并不在于1组I/O的处理速度，而是在于多组I/O的处理性能上
    
5. Asynchronous I/O 模型
    用户进程发起read操作后，立刻就可以去做其他事情
    同时从kernel角度，当它收到一个asynchronous read后，首先它会立刻返回，所以不会对用户进程产生任何block
    然后，kernel会等待数据准备完成，然后主动将数据拷贝到用户内存（“主动”这个词很重要！）
    当这一切都完成之后，kernel会给用户进程发送一个signal，告诉它read操作完成了

    所以，Asynchronous I/O模型的特点就是在I/O执行的两个阶段都没有被block
    而且期间用户进程不需要去查询I/O执行的状态，也不需要主动去拷贝数据

6. 各个I/O模型的比较图
    blocking    |non-blocking   |I/O multiplexing   |signal-driving I/O |asynchronous   
    ---------------------------------------------------------------------------------
    发起        |   查询        |   查询            |                   |发起
     |          |   查询        |    |              |                   |
     |          |   查询        |    |blocked       |                   |
     |          |   查询        |    |              |   收到signal      |
     |blocked   |   查询        |   准备完毕        |   准备完毕        |
     |          |    |          |   发起            |   发起            |
     |          |    |          |    |              |    |              |
     |          |    |blocked   |    |blocked       |    |blocked       |
     |          |    |          |    |              |    |              |收到signal
    完成        |   完成        |   完成            |   完成            |完成

7. 阻塞/非阻塞、同步/异步模型总结    
    [1]. 只有用户进程才有阻塞/非阻塞、同步/异步这些说法
    [2]. 除了Asynchronous I/O模型，其余3种模型都属于同步I/O模型
        同步I/O定义（POSIX）：A synchronous I/O operation cause the requesting process to be blocked until that I/O operation completes
        异步I/O定义（POSIX）：An asynchronous I/O operation does not cause the requesting process to be blocked
        所以，两者的区别就在于“I/O operation”时（包含上面的2个阶段），是否会将用户进程block，只要有一个阶段被block，就是同步I/O
    [3]. 异步I/O模型必定是非阻塞的，只有同步I/O模型才有阻塞/非阻塞之分，除了Non-blocking I/O模型，剩下2种模型都属于阻塞模型
        阻塞I/O定义（自定义）：I/O执行的两个阶段都被block
        非阻塞I/O定义（自定义）：I/O执行的第一阶段没有被block



