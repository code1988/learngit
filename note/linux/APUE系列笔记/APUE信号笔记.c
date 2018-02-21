1. 信号的本质
    信号是在软件层次上对中断机制的一种模拟，是IPC机制中唯一的异步通信机制.

    linux支持1～31号非实时信号，以及SIGRTMIN(通常就是32)～SIGRTMAX(通常就是64)的实时信号，需要注意的是，不存在编号为0的信号

    进程本身是无法直接处理信号的，只能通过告诉内核对指定信号的处理方式，由内核代为处理，处理方式可以分为三种：
        [1]. 忽略此信号(一定要注意，是进程忽略指定信号，这种情况下内核可能会执行一些额外的处理!)。
        [2]. 捕捉信号。这种方式需要用户提供一个相应的信号处理函数
        [3]. 执行该信号的默认动作(一定要注意，如果该信号的默认动作是"忽略"，那是内核将忽略发往指定进程的指定信号，此时内核是不对该信号做额外处理的!)
    需要注意的是，SIGKILL和SIGSTOP这2种信号既不能被忽略，也不能被捕捉，因为这2种信号需要用来确保进程一定被终止

