							Linux的init进程
/*****************************************************************************************
	init是内核启动完毕后，启动的第一个进程，进程编号始终为1，一般位于/sbin/init，系统所有进程的父进程
	init进程启动时，会读取/etc/inittab文件中的内容，这个文件是init进程的配置文件（inittab配置文件PC平台和嵌入式平台有区别）
	
	2.1 PC平台上init执行流程（基于旧的sysvinit系统启动进程管理体系）：
		执行inittab配置文件中指定的执行初始化脚本/etc/rc.d/rc.sysinit(老的系统可能是/etc/init.d/rcS)
		执行脚本/etc/rc.d/rc，根据inittab配置文件中指定的initdefault X，运行对应目录/etc/rc.d/rcX.d/下的各个程序
		执行最后一个初始化脚本/etc/rc.d/rc.local，用户可以在这个脚本中添加开机需要启动的命令
						
		备注：目前Fedora 14以后，init已经开始基于新的systemd系统启动进程管理体系，inittab文件除了设置initdefault外，只具有提示意义了	 
	
	2.2 嵌入式平台上init执行流程
		执行inittab配置文件中指定的执行初始化脚本/etc/init.d/rcS
							
*****************************************************************************************/

							PC平台Linux运行级别概念
/*****************************************************************************************
	[root@code /]#	runlevel/who -r		-	查看当前的运行级别
					init 3				-	切换到指定的运行等级
					
	7个运行等级：	0	-	系统停机状态
					1	-	单用户工作状态
					2	-	多用户状态（没有NFS）
					3	-	字符界面状态（完全多用户状态，有NFS）
					4	-	未使用
					5	-	图形界面状态
					6	-	重启
	
	开机启动时运行等级设置：/etc/inittab
	备注：不要把inittab文件中的运行级别initdefault设为0或6
*****************************************************************************************/

							嵌入式Linux的inittab文件解析
/*****************************************************************************************
	以下是S3C2416上的inittab文件：
		::sysinit:/etc/init.d/rcS
		console::askfirst:-/bin/sh
		::ctrlaltdel:-/sbin/reboot
		::shutdown:/bin/umount -a -r
		::restart:/sbin/init			
		
		每行的格式如下
		<id>:<runlevel>:<action>:<process>
		id：		嵌入式Linux里，这个字段不是随意定义的，因为头部会默认加上/dev/，也就是说该字段必须是/dev/目录下的文件名
		renlevel:：	嵌入式Linux里，这个字段被忽略，也就是说，init不支持运行级别
		action：	这个字段用于描述后面的process，有以下几个固定值
					sysinit				- 	表明在系统启动阶段，后面的process将被执行，这里就是执行/etc/init.d/rcS脚本
					askfirst/respawn	- 	两个类似，都表明后面的process进程结束时，又会被重启
											区别在于askfirst在运行process之前，会打印“Please press Enter to activate this console”，等用户按下Enter来启动该process
											这里表明/bin/sh会在用户按下Enter键后，在/dev/console这个终端上被执行
					ctrlaltdel			-	表明当Ctrl+Alt+Del三个键同时按下后，init进程就会受到SIGINT信号，此时运行process
					shutdown			- 	表明在系统关机时执行process，这就是卸载所有已挂载的设备
					restart				-	表明重新启动init进程时执行process，这里就是/sbin/init
					wiat				-	表明init进程会等到该process执行完毕，然后执行下一项（这种线性单线程启动方式是导致开机慢的一个原因）
					once				-	表明process只会执行一次，而且init进程不会等待它完成
		process：	该字段表示要执行的程序和相应的参数，或者是脚本
					
*****************************************************************************************/
					