1. 异步信号安全函数又被称为可重入函数，其定义源自单线程环境：
        如果一个函数可以在执行期间的任意时刻被中断，并且可以在中断服务程序中再次被安全调用，以及在中断退出后还可以继续正确执行完函数的剩下部分内容，就称该函数为可重入函数。
   
    1.1. 常用的可重入函数主要可以分为以下几类：
        [1]. 所有套接字API，包括socket、connect、bind、accept、listen、select、pselect、poll、recv、recvfrom、recvmsg、send、sendto、sendmsg、setsockopt、getsockopt、socketpair
        [2]. 绝大部分非标准库文件I/O函数，包括read、write、open、openat、creat、close、lseek、dup、dup2、fsync、fdatasync(缺sync)、stat、fstat、fstatat、lstat、access、faccessat、
             umask、chmod、fchmod、fchmodat、chown、fchown、fchownat、link、linkat、unlink、unlinkat(缺remove)、futimens、utimensat、utimes、mkdir、mkdirat(缺rmdir)、chdir(缺fchdir)
        [3]. 所有进程信号API，包括signal、kill、raise、alarm、pause、sigprocmask、sigpending、sigaction、sigsuspend、abort、sleep(缺nanosleep和clock_nanosleep)、sigqueue
        [4]. 绝大部分进程控制API，包括fork(缺vfork)、_exit、_Exit(缺exit)、wait、waitpid、execl、execle、execv、execve、setuid、setgid、setsid、setpgid、getuid、geteuid、getgid、getegid、getpid、getppid
        [5]. clock_gettime、time
        [6]. POSIX函数sem_post

    1.2. 不可重入函数通常会符合以下这些特征之一：
        [1]. 函数中使用了静态数据结构
        [2]. 函数中使用了malloc或free
        [3]. 标准库I/O函数

2. 线程安全函数的定义来自多线程环境：
        如果一个函数在相同的时间点可以被多个线程安全的调用，就称该函数为线程安全函数。

    2.1. POSIX中大多数函数都是线程安全的，常用的非线程安全函数主要有以下这些：
        [1]. basename、dirname、readdir
        [2]. getenv、setenv、unsetenv、system、
        [3]. gethostent、getnetbyaddr、getnetbyname、getnetent、getserverbyname、getserverbyport、getservent
        [4]. strok
        [5]. strerror、strsignal
        [6]. getopt
        [7]. localtime、gmtime、
        [8]. rand

3. 异步信号安全(可重入)函数和线程安全函数之间没有任何必然联系，也就是说：
    [1]. 一个线程安全函数不一定是可重入函数
        很多线程安全函数中都使用了共享数据，内部通过使用一系列pthread同步措施确保了同一时刻可以被多个线程安全调用，但这些同步措施并不能保证在中断中可以安全重入，因为pthread函数本身就不能保证是可重入的。
        典型的比如malloc是线程安全函数但不是可重入函数，因为它维护了一张全局链表。
    [2]. 一个可重入函数不一定是线程安全函数
        虽然实际应用中可重入函数通常也是线程安全函数，但其实并不绝对，wikipedia上就举了一个例子:
        int t;
        void swap(int *x, int *y)
        {
            int s;

            s = t; // save global variable
            t = *x;
            *x = *y;

            //hardware interrupt might invoke isr() here!
            *y = t;
            t = s; // restore global variable
        }
        void isr()
        {
            int x = 1, y = 2;
            swap(&x, &y);
        }
        swap函数开始处将全局变量t保存到了本地变量，最后在函数返回前重新恢复全局变量t，这使得swap函数称为可重入函数，但显然不是线程安全函数。  
        

