记录锁提供的功能： 
        当第一个进程正在访问文件的某个部分时，使用记录锁可以限制其他进程访问同一文件区的行为
        这里需要注意一点，记录锁支持锁定文件的一个区域（当然也可以是整个文件）


fcntl记录锁接口格式：
        int fcntl(int fd,int cmd,struct flock *flockptr)
        @fd         - 要设置记录锁的文件描述符
        @cmd        - 对于记录锁，可以取F_GETLK、F_SETLK、F_SETLKW
        @flockptr   - flock结构如下：
                      struct flock {
                          short l_type;     // 本次记录锁操作类型，可以取F_RDLCK(上读锁)、F_WRLCK(上写锁)、F_UNLCK(解锁)
                          short l_whence;   // 本次记录锁操作基准位置，可以取SEEK_SET、SEEK_CUR、SEEK_END
                          off_t l_start;    // 本次记录锁操作相对基准位置的偏移量
                          off_t l_len;      // 本次记录锁操作字节长度
                          pid_t l_pid;      // 用于存放当前持有该记录锁的进程ID，本字段只在F_GETLK情况下有意义
                      };
                      这里需要注意几点：
                      [1]. 锁可以在文件尾端之后开始，但不能在文件起始位置之前开始
                      [2]. l_len取0则表示锁的范围会自动覆盖最大可能偏移量，这意味着不管向该文件追加写了多少数据，这些数据都处于锁的范围内
                      [3]. 对整个文件加锁的常用方法就是: l_whence取SEEK_SET，l_start取0，l_len取0

记录锁又分为两种类型，共享读锁和独占性写锁。其使用规则如下：
        [1]. 任意多个进程在文件
