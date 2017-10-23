1. 信号的本质
    信号是在软件层次上对中断机制的一种模拟，是IPC机制中唯一的异步通信机制.

    进程本身是无法直接处理信号的，只能通过告诉内核对指定信号的处理方式，由内核代为处理，处理方式可以分为三种：
        [1]. 忽略此信号(一定要注意，是进程忽略指定信号，这种情况下内核可能会执行一些额外的处理!)。
        [2]. 捕捉信号。这种方式需要用户提供一个相应的信号处理函数
        [3]. 执行该信号的默认动作(一定要注意，如果该信号的默认动作是"忽略"，那是内核将忽略发往指定进程的指定信号，此时内核是不对该信号做额外处理的!)
    需要注意的是，SIGKILL 和 SIGSTOP 这2种信号既不能被忽略，也不能被捕捉，因为这2种信号需要用来确保进程一定被终止

2. 几种常用的信号分析
    [1]. SIGCHLD 
        一个进程结束时，SIGCHLD 信号由内核发送给其父进程，但是在父进程不针对该信号处理做显式指定的情况下，内核默认将直接丢弃该信号(这就是僵尸进程产生的根源)。
        所以对 SIGCHLD 信号正确的处理方式是：
                如果父进程不关心子进程结束时的状态信息，则可以事先通知内核忽略该信号，那么子进程结束时，内核会负责回收，并不再给父进程发信号;
                如果父进程希望被告知子进程结束时的状态信息，则应捕捉此信号，并且信号处理函数中要调用一种wait函数来取得子进程的ID和终止状态
    [2]. SIGPIPE
        两种情况下会产生该信号：
                在管道的读进程已经终止时写管道;
                在流式套接字的连接断开时写该套接字，由于这种情况更加普遍，这里加以具体分析:
                        根据TCP/IP协议，当对端调用close时，本端收到FIN包，这时对本端调用第一次write，会收到一个RST响应
                        调用第二次write，系统会发出SIGPIPE信号给进程
        SIGPIPE信号的默认处理规则是终止，所以导致进程退出，为了避免进程退出，则可以事先通知内核忽略该信号，
        这样在对管道调用write时(对流式套接字第二次调用write时)，会返回-1,同时errno置为SIGPIPE,程序便能知道对端已经关闭
    [3]. SIGUSR1 SIGUSR2
        这两个都是预留给用户自定义用途的信号，默认动作都是终止进程
    [4]. SIGTERM
        这是kill命令缺省发送的系统默认的进程终止信号，用户进程通常会捕获该信号，完成结束之前的清理工作

3. 可靠信号和不可靠信号
    linux支持1～31号非实时信号(也就是不可靠信号)，以及SIGRTMIN(通常就是32)～SIGRTMAX(通常就是64)的实时信号(也就是可靠信号)，需要注意的是，不存在编号为0的信号

    linux对不可靠信号的机制做了一点改进：在调用完信号的处理函数后，不会将该信号重置成默认动作。
    所以在linux上，不可靠信号存在的问题主要就是信号可能会丢失。

    由于早期定义的不可靠信号不好再做改动，所以只好新增加了SIGRTMIN~SIGRTMAX数量的信号，这些可靠信号支持排队，因而不会丢失

4. 信号安装和检查 
    有2个函数可以实现信号的安装，分别是signal和sigaction(新的linux平台上signal实际就是通过sigaction函数实现的).

    以下是signal函数相关定义：
        typedef void (*sighandler_t)(int);
        sighandler_t signal(int signo,sighandler_t handler);
        @signo      信号ID(除了SIGSTOP和SIGKILL)
        @handler    SIG_IGN/SIG_DFL/自定义的信号捕捉函数地址
        @返回值     返回该信号旧的处理函数的地址
    signal函数的缺陷:
        signal函数的语义跟实现有关，即便同是在linux上，各个版本之间也存在差异;
        无法通过signal函数在不改变一个信号的处理方式的情况下获取该信号当前的处理方式(获取方式详见APUE-P325);

    所以最好使用sigaction来代替signal，以下是sigaction函数相关定义:
        int sigaction(int signo,const struct sigaction *act,struct sigaction *oact);
        @signo      信号ID(除了SIGSTOP和SIGKILL)
        @act        非NULL时表示为指定信号安装新的处理动作
        @oact       非NULL时用来存放指定信号旧的处理动作(如果传入的act同时为NULL，就实现了查询该信号当前的处理方式的功能)；如果不关心旧状态就设为NULL
        该函数用到了struct sigaction结构(具体格式跟平台相关)：
        struct sigaction {
            union {
                void (*sa_handler)(int);                        // SIG_IGN/SIG_DFL/自定义的信号捕捉函数地址 
                void (*sa_sigaction)(int,siginfo_t *,void *);
            }_u;
	        sigset_t sa_mask;
            int sa_flags;
        }
