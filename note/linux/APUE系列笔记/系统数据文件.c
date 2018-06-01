1. 口令文件(/etc/passwd 644)
    口令文件记录了linux用户账户以及相关信息（密码除外），每一行中各字段定义如下：
    <pwd.h>
    struct passwd {
        char *pw_name;  // 用户名
        char *pw_passwd;// 加密口令
        __uid_t pw_uid; // 用户ID
        __git_t pw_gid; // 组ID
        char *pw_gecos; // 注释字段
        char *pw_dir;   // home目录
        char *pw_shell; // 该用户的登陆shell
    }
    备注：通常有一个用户名为root的登陆项，其用户ID是0，也就是超级用户；通常还有一个用户名为nobody的登陆项，其用户ID是65534，一般对该用户不提供任何特权，只能访问人人都可读写的文件
          加密口令字段在这里就是个占位符"x"，实际的加密口令放到了/etc/shadow中
          用户ID和组ID应该就是当前进程的实际用户ID和实际组ID
          某些字段可以为空，注释字段为空不产生任何影响
          home目录字段记录的就是该用户的家目录
          shell字段一般指定了一个shell程序用来登陆系统，这里有多种方法来组织一个特定用户登陆系统，常见的有/dev/null、/bin/false、/bin/true、/usr/sbin/nologin等

2. 阴影口令(/etc/shadow 640)
    阴影口令文件主要记录了用户账户对应的加密口令，每一行中各字段定义见<shadow.h>，这里不再展开

    备注：除了root用户，其他用户对阴影口令文件没有访问权限，从而确保了加密口令不会被泄漏,而普通口令文件是可以被各用户自由读取的
    
3. 组文件(/etc/group 644)
    组文件记录了linux用户组以及相关信息，每一行中个字段定义见<grp.h>，这里不在展开
    备注：组文件的权限以及访问的C接口风格跟口令文件基本相同

4. 账户添加/删除命令
    只有root权限才能添加/删除账户。
    [1]. "# useradd 用户名"     - 这条命令只是创建了一个用户，它并没有在/home目录下创建同名文件夹，也没有创建密码，所以使用该用户名是无法登录系统的
    [2]. "# useradd -m 用户名"  - 这条命令不但创建了一个用户，还会在/home目录下创建同名文件夹
    [3]. "# passwd 用户名"      - 这条命令用于对已经创建的用户设置密码，如果是针对本用户操作则不需要root权限
    [4]. "# adduser 用户名"     - 这条perl命令是对useradd、passwd等命令的封装，所以简单的使用该条命令就可以完成账户创建和密码设置等
    [5]. "# userdel -r 用户名"  - 这条命令用于删除一个已经创建的用户，其中-r表示同时删除该用户的home目录等相关文件
    
5. 其他系统数据文件
    /etc/hosts      - 主机名配置文件
    /etc/networks   - 用来记录网络信息
    /etc/protocols  - 用来记录协议信息
    /etc/services   - 用来记录各网络服务器所提供的服务
    备注：这些文件对具体linux系统来说不一定是必需的

6. utmp和wtmp文件
    /var/run/utmp文件记录了当前登录到系统的各个用户；/var/log/wtmp文件记录了每一次登录和注销事件
    who命令的实现就是基于utmp文件，只是在内容的展现上根据登录终端进行了细分
    last命令的实现基于wtmp文件

7. 时间信息总结
    linux采用日历时间(计算从1970年1月1日00:00:00以来经过的秒数，数据类型 time_t)来提供基本的时间服务
    [1]. time_t time(time_t *time)      -- <time.h>
        最基本的时间获取函数，时间值既可以通过入参获取，也可以通过返回值获取

    [2]. struct tm *localtime(const time_t *time)   -- <time.h>
         struct tm *gmtime(const time_t *time)
         time_t mktime(struct tm *time)
        localtime是将日历时间转换为年月日时分秒格式的本地时间，要注意的是，转换出来的年份值tm_year + 1900等于当前年份，月份值tm_mon + 1 等于当前月份；
        gmtime也是将日历时间转换为年月日时分秒格式，但是没有经过时区变换
        mktime则是将年月日时分秒格式时间转换为日历时间

    [3]. char *asctime(const struct tm *time)   -- <time.h>
         char *ctime(const time_t *time)
        asctime将struct tm格式的日期时间转换成格式化的字符串形式真实日期，转换中自动处理了年份和月份的偏移量
        ctime将time_t格式的秒数转换为格式化的字符串形式真实日期

    [4]. int gettimeofday(struct timeval *tv,void *tz)  -- <sys/time.h>
        gettimeofday将日历时间精确到微秒级别
        tz唯一的合法值是NULL，其他值将产生不确定的结果
        POSIX.1-2008指定该函数已经废弃，由clock_gettime代替

    [5]. int clock_gettime(clockid_t clock_id,struct timespec *time)   -- <sys/time.h>
        clock_gettime将获取的时间进一步精确到纳秒级别
        clock_id用来选择不同的时钟基准，从而使该函数可以获取不同的系统时间
                CLOCK_REALTIME  - 从1970-1-1 0：0：0开始计时，所以获取的时间表示精确到纳秒级别的日历时间
                CLOCK_MONOTONIC - 从系统启动开始计时，所以获取的时间表示系统运行时间
                CLOCK_PROCESS_CPUTIME_ID    - 从本进程启动开始计时，所以表示本进程的运行时间
                CLOCK_THREAD_CPUTIME_ID     - 从本线程启动开始计时，所以表示本线程的运行时间





