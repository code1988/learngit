                                            进程环境
----------------------------------------------------------------------------------------------------
1. 环境变量
    每个进程在被创建的时候都会收到一张命令行参数表和一张环境变量表，通常两张表都位于本进程存储空间的顶部（栈之上）
    
    获取环境变量的方法：
        char *getenv(const char *name);     // 函数返回一个指针，指向name=value字符串中的value
    修改环境变量的方法：
        int setenv(const char *name,const char *value,int rewrite); // 开辟一块堆内存，放入name=value字符串.如果环境中name已经存在，则根据rewrite值决定是否进行覆盖
        int putenv(char *str);                                      // 将形式为name=value的字符串地址(字符串空间不能是在栈上分配的)直接放到环境表中，如果name已经存在，则先删除原来的定义
        int unsetenv(const char *name);                             // 删除name的环境变量，即使不存在也不返回错误
    
2. 非局部跳转
    不同于普通C语言的goto语句在一个函数内进行跳转，非局部跳转可以在栈上进行跨函数跳转，从而返回到当前函数调用路径上的某个函数中。通过2个函数来实现：
            int setjmp(jmp_buf env);            // 直接调用时返回0；从longjmp调用时返回longjmp传入的val
            void longjmp(jmp_buf env,int val);  // 调用该函数可以返回到上次执行setjmp的位置
    参数env包含了恢复栈状态的所有信息，由setjmp生成导出，由longjmp导入恢复，所以参数env必须是全局性质的;
    根据返回值的不同，一个setjmp可以对应多一个longjmp，因为setjmp的返回值对应了longjmp传入的val.
----------------------------------------------------------------------------------------------------

                                            进程控制
----------------------------------------------------------------------------------------------------
1. 进程ID
    每个进程都由一个非负整数来唯一标识，这就叫进程ID。
    ps -axj命令输出的进程中:
            ID为1的进程通常是init进程，内核在完成初始化后就会调用该进程进一步执行具体系统(比如debian)的初始化，不同于0号进程，init进程属于用户态进程，并且拥有root权限;
            内核进程的名字出现在方括号中(0号进程通常是调度进程，属于内核的一部分，所以不直接显示在ps命令中)

2. 进程创建
    linux下主要实现了3种进程创建函数：
        [1]. pid_t fork(void)
        作为标准的进程创建方式，fork创建的子进程会获得父进程数据段、栈、堆的副本，只有正文段始终是共享的。
        以上这种早期的fork实现方式存在执行效率低下的问题，所以linux采用了COW技术:
                在fork时并不执行对父进程数据段、栈、堆的拷贝，而是父子进程共享这些区域(实际就是共享原本只属于父进程的物理页面)，并且这些被共享区域的访问权限被改为只读。
                当父进程和子进程中的任何一个试图修改这些共享的物理页面时，就产生一个page_fault,内核在异常处理函数中会对这块导致page_fault的物理页面取消共享，并为写进程
                分配新的物理页面并标记为可写(至于原来的物理页面如果内核判定只有一个属主时，也会将其访问权限改回可写)，这样父子进程就各自拥有一块内容相同的物理页面。
        需要注意的一点就是以上这种COW技术按"页"为单位实施，所以更具灵活性。

        [2]. pid_t vfork(void)
        首先要明确一点，vfork早在4.4BSD中就被移除，并且POSIX.1-2001中被标记为作废，POSIX.1-2008中移除了操作说明，这些都意味着尽量别再使用vfork了。
        同样是创建一个新进程，vfork跟fork相比，主要的区别有以下几点：
                不同于fork后父子进程的执行顺序是不确定的，vfork保证子进程先运行，只有当子进程调用exec/_exit之后，父进程才会恢复运行。
                采用COW技术的fork只是共享了物理地址，父进程的虚拟地址空间还是会完全复制一份给子进程，而vfork出来的子进程直接运行在父进程的虚拟地址空间(共享虚拟地址和物理地址空间)。
                这种激进的做法虽然进一步提高了执行效率，但函数本身的设计瑕疵导致了一个问题，那就是如果子进程修改数据、调用函数、或者没有调用exec/_exit
                就返回，这些操作都会带来未知后果。
        总而言之，使用vfork的通常情况是后面紧跟着exec操作。

        [3]. int clone(int (*fn)(void),void *child_stack,int flags,void *arg,...)
        
3. 进程终止
    有5种方式可以使进程正常终止：
                [1]. 从main返回
                [2]. 调用exit
                [3]. 调用_exit或_Exit
                [4]. 最后一个线程从其启动例程返回
                [5]. 最后一个线程调用pthread_exit

    exit/_exit/_Exit这3个函数的区别：
                [1]. _exit和_Exit立即进入内核,而exit则先执行一些清理动作(包括调用执行事先注册了的终止处理函数，冲洗标准I/O流，fclose所有打开的标准I/O流等)，然后调用_exit/_Exit
                [2]. exit和_Exit是ISO C定义的，意味着是标准C库函数，头文件是stdlib.h，而_exit是由POSIX.1定义的，头文件按是unistd.h
                                     
    atexit函数用于注册终止处理函数，原型如下：
        int atexit(void (*func)(void))
    这些终止处理函数将由exit自动调用，exit调用它们的顺序和它们被注册时的顺序正好相反，也就是第一个注册的终止处理函数将会被最后调用

    有3种方式可以使进程异常终止：
                [1]. 调用abort函数
                [2]. 接收到某些信号
                [3]. 最后一个线程对"取消"请求作出响应

    有一点需要注意，不管进程如何终止，都会执行内核中的同一段代码，这段代码会负责关闭该进程打开的所有文件描述符，所以现在实际的exit实现中，可能不再多此一举(但是exit一定会实现冲洗操作)
                
4. wait系列函数
    父进程是通过wait系列函数来主动获取子进程的终止状态。
        [1]. pid_t wait(int *state)
             @state  - 如果调用时传入非空指针，则终止子进程的终止状态会存入state指向的空间；如果传入空指针，意味不关心终止状态
             @返回值 - 成功返回终止进程的进程id，出错时返回-1
        当调用wait时，如果还没有子进程终止，则阻塞；如果有任何一个子进程已经终止成为僵尸进程，则取得该子进程终止状态并立即返回；如果根本就没有任何子进程存在，则立即返回-1。

        [2]. pid_t waitpid(pid_t pid,int *state,int opt)
             @pid   - pid == -1，等待任一子进程，这种情况下跟wait等效；
                    - pid == 0，等待进程组ID等于调用进程组ID的任一子进程；
                    - pid > 0，等待进程ID和pid相等的子进程；
                    - pid < -1，等待进程组ID等于pid绝对值的任一子进程
             @state - 基本类似wait中
             @opt   -   WCONTINUED  : 跟作业控制相关，略
                        WNOHANG     : 设置waitpid为非阻塞，意味着指定pid如果还未终止，则返回0
                        WUNTRACED   : 跟作业控制相关，略
             @返回值- 基本类似wait(差别在于-1的原因)
        
        [3]. int waitid(idtype_t idtype,id_t id,siginfo_t *infop,int options)

        [4]. wait3/wait4(略)

    wait系列函数中用来记录终止状态的state包含了多类具体信息，所以实际通过调用4个互斥的系统定义宏(位于sys/wait.h中)来判断具体的终止原因：
            WIFEXITED(state)    - 为真，首先表示子进程是正常终止，然后进一步调用WEXITSTATUS(state)获取子进程exit/_exit时传入的值（低8位）
            WIFSIGNALED(state)  - 为真，首先表示子进程是异常终止，然后进一步调用WTERMSIG(state)获取使子进程终止的信号id，以及调用WCOREDUMP(state)判断是否产生了core文件
            WIFSTOPPED(state)   - 为真，首先表示子进程被暂停，然后进一笔调用WSTOPSIG(state)获取是子进程暂停的信号id
            WIFCONTINUED(state) - 作业控制相关，略

5. exec系列函数
    进程通过调用exec系列函数来加载并运行一个新的程序。
    要注意的是，exec系列函数只是用新程序的正文段、数据段、堆、栈全面覆盖掉了原有进程中的相关内容，这意味着它并不创建新的进程，所以前后的进程ID不变。
    exec系列函数通过以下几个标识符的组合来定义：
            "l"就是list(l和v互斥)，表示新程序的每个命令行参数以字符串的形式独立地作为exec的入参(const char *arg0,...)，并且最后一个入参以空指针结尾
            "e"就是environment，表示可以传递一个指向环境字符串指针数组的指针，用于手动为子进程指定一个环境
            "p"就是path，表示使用新程序的文件名作为exec的入参，调用进程会按照PATH环境变量所指定的目录去搜寻匹配的文件名
            "v"就是vector(l和v互斥)，表示新程序的每个命令行参数以字符串指针的形式存入一个统一的数组中，然后将该数组作为exec的入参(char *const argv[])
            "f"就是fd，表示使用文件描述符作为exec的入参，调用进程使用文件描述符去执行对应的文件
    经过组合得到的exec系列函数如下：
            [1]. int execl(const char *pathname,const char *arg0,.../*(char *)0*/)                      - "+l"
            [2]. int execlp(const char *filename,const char *arg0,.../*(char *)0*/)                     - "+l +e"
            [3]. int execle(const char *pathname,const char *arg0,.../*(char *)0*/,char *const envp[]/)  - "+l +e"

            [4]. int execv(const char *pathname,char *const argv[])                         - "+v"
            [5]. int execvp(const char *filename,char *const argv[])                        - "+v +p"
            [6]. int execve(const char *pathname,char *const argv[],char *const envp[])     - "+v +e"
            [7]. int fexecve(int fd,char *const argv[],char *const envp[])                  - "+v +e +f"
