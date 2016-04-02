1. ${parameter=default},${parameter:=default}详解
	${parameter=default}	- 如果变量没有被声明，那就使用默认值
	${parameter:=default}	- 如果变量没有被设置，那就使用默认值
	备注：null是指该变量已经存在（内存地址上），但其值是空的，也就是已经被声明，但没有被设置
		  unset是删除变量（连同内存地址都释放），也就是该变量不存在了
	例.
		# parameter属于unset那种情况，意味着没有被声明，更谈不上被设置
		$ var=${str=1}			$ var=${str:=1}
		$ echo $str $var		$ echo $str $var
		# 这种情况下，两种处理结果相同：使用默认值1
		$	1 1					$	1 1		

		# parameter属于null那种情况，意味着被声明但没有被设置
		$ str					$ str
		$ var=${str=1}			$ var=${str:=1}
		$ echo $str $var		$ echo $str $var
		# 因为变量已经被声明，采用“=”的不使用默认值1,采用“:=”的使用默认值1
		$						$	1 1
	
		# parameter属于非空值那种情况
		$ str=2					$ str=2
		$ var=${str=1}			$ var=${str:=1}
		$ echo $str $var		$ echo $str $var
		# 变量已经被赋值时，两种处理结果都相同：不使用默认值1
		$	2 2					$	2 2
	
