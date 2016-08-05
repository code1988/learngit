1. int scandir(const char *dir,struct dirent ***namelist,int(*filter)(const struct dirent *),int(*compare)(const struct dirent **,const struct dirent**))
---------------------------------------------------------------------------------------------------------------------------------------------------------
    释义：扫描dir目录下(不包括子目录)满足filter函数过滤模式的文件/文件夹，返回的结果会经过compare函数排序，最后保存在namelist中
    参数：
          [namelist] - 三维指针namelist指向的内存空间是由scandir内部通过malloc分配的，所以用完后需要释放，并且要释放2个纬度的内存
          [filter]   - 为NULL时表示不进行过滤,编写filter时需要过滤掉的返回0,需要保留的返回1
          [compare]  - alphasort和version是使用到的两种排序函数
    返回值：成功返回找到匹配模式文件的个数，失败返回-1
    备注：扫描到的文件中会包含"."和".."两个文件
    示例：见man scandir

2. int stat(const char *file_name,struct stat *buf)
---------------------------------------------------------------------------------------------------------------------------------------------------------
    释义：获取文件的详细信息(一般是文件没有打开时这样操作),保存在struct stat 结构中
            struct stat{
                dev_t st_dev;               // 文件的设备ID
                ino_t st_ino;               // 节点
                mode_t st_mode;             // 文件类型和权限
                nlink_t st_nlink;           // 连接到该文件的硬链接数目，刚建立的文件为1
                uid_t st_uid;               // 用户ID
                gid_t st_gid;               // 组ID
                dev_t st_rdev;              // 设备ID
                off_t st_size;              // 文件字节数
                unsigned long st_blksize;   // 块大小（文件系统I/O缓冲区大小）
                unsigned long st_blocks;    // 块数量
                time_t st_atime;            // 最后一次访问事件
                time_t st_mtime;            // 最后一次内容修改时间
                time_t st_ctime;            // 最后一次属性修改时间
            }
                            文件类型和存取权限st_mode       对应的用于检查类型的宏定义
            socket文件      S_IFSOCK                        S_ISSOCK(st_mode) 
            符号链接文件    S_IFLNK                         S_ISLNK(st_mode) 
            一般文件        S_IFREG                         S_ISREG(st_mode) 
            块设备          S_IFBLK                         S_ISBLK(st_mode) 
            目录文件        S_IFDIR                         S_ISDIR(st_mode) 
            字符设备        S_IFCHR                         S_ISCHR(st_mode) 
            FIFO文件        S_IFIFO                         S_ISFIFO(st_mode) 
    返回值：失败时错误信息见errno

3. int access(const char *pathname,int mode)
---------------------------------------------------------------------------------------------------------------------------------------------------------
    释义：判断文件权限
    参数：
          [mode] - 需要被测试的权限，取值如下
                    R_OK    测试读权限
                    W_OK    测试写权限
                    X_OK    测试执行权限
                    F_OK    测试文件是否存在
