							本地git操作相关
/****************************************************************************************
创建版本库：					# git init
添加文件到git仓库缓冲：			# git add 
写入git仓库缓冲中的文件：		# git commit -m
查看当前工作区状态：			# git status
查看具体修改内容：				# git diff
查看历史版本变更：				# git log
变更到指定的历史版本：			# git reset --hard 
查看历史命令：					# git reflog	
删除git仓库中文件:				# git rm 	+  	# git commit -m
还原git仓库中版本到当前工作区:	# git checkout
***************************************************************************************/	

							github操作相关
/****************************************************************************************
账号：code1988
密码：code504136046

1. 	由于本地git仓库和github仓库之间的传输是通过ssh加密的，所以首先创建ssh key
	# ssh-keygen -t rsa -C "504136046@qq.com"
	登录github，setting->ssh keys->add ssh key，添加本地生成的ssh key

2. 	把本地仓库与github仓库关联
	# git remote add origin git@github.com:code1988/learngit.git	或者
	# git remote add origin https://github.com/code1988/learngit.git
	关联时，需要输入github的账号密码，关联之后，远程库的名字就是origin

本地库所有内容推送到github库：	# git push (-u) origin master		// 第一次推送时需要加上-u参数，以后就不用了
把远程库克隆到本地：			# git clone git@github.com:code1988/learngit.git	或者
								# git clone https://github.com/code1988/learngit.git								
***************************************************************************************/	
											