1. 信号的本质
    信号是在软件层次上对中断机制的一种模拟，是IPC机制中唯一的异步通信机制.

    进程本身是无法直接处理信号的，而是必须事先将处理方式告诉内核，处理方式可以分为三种：
        [1]. 忽略此信号
        [2]. 捕捉信号。这种方式需要用户提供一个相应的信号处理函数
        [3]. 执行该信号的默认动作。默认动作主要有：忽略、终止、终止+core、继续
    需要注意的是，SIGKILL 和 SIGSTOP 这2种信号既不能被忽略，也不能被捕捉，因为这2种信号需要用来确保进程一定被终止

    在信号产生之后，如果进程对该信号的动作设置了忽略(进程无法阻塞被忽略的信号)，则内核直接做忽略处理；
    在信号产生之后，到指定进程确认收到该信号之前，这段期间称信号是未决的，只有进程确认收到该信号时，才会决定对它的处理方式(除了进程设置的忽略);
    如果产生了一个被阻塞的信号，并且进程对该信号的动作是默认或捕捉，则该信号将一直保持在未决状态，意味着未决期间进程仍可以改变对该信号的动作.

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
    [5]. SIGALRM
        当用alarm函数设置的定时器超时时产生此信号，默认动作是终止进程
    [6]. SIGABRT
        调用abort函数时产生此信号，使进程异常终止(缺省还会产生core文件)，需要注意的是，该信号无法被阻塞或忽略
    [7]. SIGHUP
        两种情况下会产生该信号：
                终端控制断开时，跟该终端相关连的进程(必然是会话首进程)会收到该信号(前提是该进程事先没有对其选择忽略);
                会话首进程终止时，前台进程组中的每个进程都会收到该信号(前提同上)，至于后台进程是否收到跟shell的huponexit选项设置有关

3. 可靠信号和不可靠信号
    linux支持1～31号非实时信号(也就是不可靠信号)，以及SIGRTMIN(通常就是32)～SIGRTMAX(通常就是64)的实时信号(也就是可靠信号)，需要注意的是，不存在编号为0的信号

    linux对不可靠信号的机制做了一点改进：在调用完信号的处理函数后，缺省(可通过SA_RESETHAND修改)不会将该信号重置成默认动作。
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
                void (*sa_handler)(int);                        // SIG_IGN/SIG_DFL/自定义的信号捕捉函数地址(显然不能传递除了信号ID之外的任何信息)，这是默认的信号捕捉函数字段 
                void (*sa_sigaction)(int,siginfo_t *,void *);   // 这个信号捕捉函数可以通过siginfo_t传递产生该信号的原因信息，只有当sa_flags中设置了SA_SIGINFO标志时才会启用该字段
            }_u;
	        sigset_t sa_mask;   // 该信号的阻塞集(只在信号捕捉函数运行期间生效)，缺省情况下，触发信号捕捉函数的该信号会被自动添加进阻塞集中，从而避免了同种信号嵌套触发
            int sa_flags;       // 修改信号行为的一系列选项集合
        }
        其中sa_flags支持的几个常用选项如下：
        SA_SIGINFO      该选项会使sa_sigaction字段生效
        SA_NODEFER      该选项会使系统执行指定信号的捕捉函数期间不自动阻塞该信号(除非手动在sa_mask中包括了该信号)，另外，相同含义的SA_NOMASK已经作废
        SA_INTERRUPT    该选项会使由此信号中断的系统调用不自动重启动(这也是目前linux平台上的缺省处理方式)
        SA_RESTART      该选项会使由此信号中断的系统调用自动重启动
        SA_RESETHAND    该选项会使系统执行指定信号的捕捉函数后，重置该信号的处理方式为 SIG_DFL
        信号捕捉函数sa_sigaction中用到了siginfo_t结构(具体格式跟平台相关)，以下只罗列了部分常用字段:
        typedef struct siginfo {
            int si_signo;           // 信号ID
            int si_code;            // 信号产生的原因码，跟具体信号ID关联，具体的原因码详见APUE-P281
            union sigval si_value;  // 这个字段主要由用户进程调用sigqueue函数传递过来，可以是一个int变量或void指针
        }siginfo_t;

    信号是否可靠只跟信号ID有关，与信号的安装函数无关，目前linux上的两个信号安装函数signal和sigaction都不能把SIGRTMIN之前的信号变成可靠信号，但都对SIGRTMIN之后的信号支持排队

5. 信号发送和捕捉
    [1]. kill函数用于将信号发送给进程或进程组(也可以发给自身)，其定义如下：
         int kill(pid_t pid,int signo)
         @pid   >0时，表示将信号发送给进程ID为pid的进程
                ==0时，表示将信号发送给与发送进程在同一进程组的所有进程(发送进程需要有权限向这些进程发送信号，并且所有进程不包括系统进程集)
                ==-1时，表示将信号发送给所有进程(发送进程需要有权限向这些进程发送信号，并且所有进程不包括系统进程集)
                <-1时，表示将信号发送给进程组ID等于pid绝对值的所有进程(发送进程需要有权限向这些进程发送信号)
    [2]. raise函数专门用于向自身进程发送信号，其定义如下：
         int raise(int signo)
    [3]. alarm函数用于向自身进程设置一个闹钟时间，超时时产生 SIGALRM 信号，其定义如下：
         unsigned int alarm(unsigned int seconds)
         @seconds   超时时间，相对值，单位s
         @返回值    0或者上次设置的闹钟时间的余留秒数    
         如果想捕捉 SIGALRM 信号，则必须在调用alarm之前安装该信号的捕捉函数(如果晚于alarm函数安装，则下次 SIGALRM 信号执行默认动作)
    [4]. abort函数用于向自身进程发送 SIGABRT 信号，使进程异常终止，其定义如下：
         void abort(void)
         如果事先注册了 SIGABRT 信号的捕捉函数，则调用abort函数后就会执行该捕捉函数，并且当捕捉函数结束后，abort函数不会再返回到其调用者
    [5]. sigqueue函数主要用于将实时信号发送给单个进程(也支持发送非实时信号)，其定义如下：
         int sigqueue(int pid_t pid,int signo,const union sigval value)
         @value     如果指定信号启用了sa_sigaction格式的信号捕捉函数，则该字段将会传递到siginfo结构的si_value字段中
    [6]. pause函数可以使调用进程挂起直至捕捉到一个信号，其定义如下：
         int pause(void)
         只有执行完一个信号处理之后，pause才返回，返回值为-1,errno设置为 EINTR

6. 信号集(sigset_t)
    信号集就是用来描述信号的集合，因为需要能容纳所有的信号，显然无法用一个整形量来表示，为此POSIX定义了数据类型sigset_t来表示信号集，并定义了以下处理信号集的API：
        int sigemptyset(sigset_t *set)          // 初始化信号集，清空所有信号
        int sigfillset(sigset_t *set)           // 初始化信号集，设置所有信号
        int sigaddset(sigset_t *set,int signo)  // 将指定信号添加到信号集中
        int sigdelset(sigset_t *set,int signo)  // 从信号集中删除指定信号
        int sigismember(const sigset_t *set,int signo)  // 检查指定信号是否包含在信号集中

    每个进程都存在一个信号屏蔽集，该信号集中的所有信号都将被阻塞而不能递送给进程，以下是跟信号屏蔽集相关的API：
        int sigprocmask(int how,const sigset_t *set,sigset_t *oset)     // 检测/修改进程当前的信号屏蔽集(仅适用于单线程的进程)
        @how    用来指示将如何修改信号屏蔽集(只在oset非NULL时有意义)，可选项如下：
                        SIG_BLOCK   - 在进程当前的信号屏蔽集中添加set信号集中的信号，这时set包含了希望阻塞的附加信号
                        SIG_UNBLOCK - 在进程当前的信号屏蔽集中删除set信号集中的信号，这时set包含了希望解除阻塞的信号
                        SIG_SETMASK - 更新进程当前的信号屏蔽集为set信号集
        @set    非NULL时，set信号集的含义跟how取值有关；为NULL时则不改变进程当前的信号屏蔽集
        @oset   进程当前的信号屏蔽集通过oset返回
        在调用sigprocmask后如果有任何未决的、不在阻塞的信号，则在sigprocmask返回前，至少将其中之一递送给该进程;
        对于处于阻塞状态的小于SIGRTMIN的不可靠信号，由于不支持排队，所以调用sigprocmask解除阻塞之后，只会向进程递送一次该信号.

        int sigpending(sigset_t *set)           // 获得当前进程中因为阻塞而不能被递送(也就是处于未决状态)的信号集
        @set    当前进程中的未决信号集通过set返回

        int sigsuspend(const sigset_t *set)     // 临时替换进程当前的信号屏蔽集，并使进程休眠，直到收到信号为止，sigsuspend返回后将恢复进程当前的信号屏蔽集
        @set    需要临时屏蔽的信号集
        @返回值 总是-1,同时errno设置为EINTR

