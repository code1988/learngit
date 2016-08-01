# 最有效率的逐行读取文件的方法
function read_line(){
    while read line
    do
        echo $line
        # 添加"< /dev/null"的目的是禁止ssh从stdin读
        # eval "echo `ssh root@${line} "uptime" < /dev/null`" 
    done < ${1}
}

