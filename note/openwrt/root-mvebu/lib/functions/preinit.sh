#!/bin/sh
# Copyright (C) 2006-2013 OpenWrt.org
# Copyright (C) 2010 Vertical Communications

boot_hook_splice_start() {
	export -n PI_HOOK_SPLICE=1
}

boot_hook_splice_finish() {
	local hook
	for hook in $PI_STACK_LIST; do
		local v; eval "v=\${${hook}_splice:+\$${hook}_splice }$hook"
		export -n "${hook}=${v% }"
		export -n "${hook}_splice="
	done
	export -n PI_HOOK_SPLICE=
}

# 始化钩子列表,也就是在钩子列表中添加钩子名
boot_hook_init() {
	local hook="${1}_hook"
	export -n "PI_STACK_LIST=${PI_STACK_LIST:+$PI_STACK_LIST }$hook"
	export -n "$hook="
}

# 注册钩子函数
boot_hook_add() {
	local hook="${1}_hook${PI_HOOK_SPLICE:+_splice}"
	local func="${2}"

	[ -n "$func" ] && {
		local v; eval "v=\$$hook"
		export -n "$hook=${v:+$v }$func"
	}
}

# 从头获取指定钩子名下的钩子函数（${1}为钩子名， ${2}变量名用于保存获取到的钩子函数）
# 每成功调用一次该函数，指定钩子名下的钩子函数就少一个
boot_hook_shift() {
	local hook="${1}_hook"
	local rvar="${2}"

	local v; eval "v=\$$hook"
	[ -n "$v" ] && {
        # 如果钩子名下有钩子函数，从头获取钩子函数
		local first="${v%% *}"

        # 钩子名下的钩子函数减一个，没有了则把钩子名指向空
		[ "$v" != "${v#* }" ] && \
			export -n "$hook=${v#* }" || \
			export -n "$hook="

		export -n "$rvar=$first"
		return 0
	}

    # 如果钩子名下没有钩子函数，则直接返回1
	return 1
}

# 运行指定的钩子名对应的钩子函数集合
boot_run_hook() {
	local hook="$1"
	local func

    # 遍历钩子名对应的钩子函数集合,执行每个钩子函数
	while boot_hook_shift "$hook" func; do
        # 给每个钩子函数添加"PI_RAN_"头
		local ran; eval "ran=\$PI_RAN_$func"
		[ -n "$ran" ] || {
            # 如果添加"PI_RAN_"头后的钩子函数不存在，则为加头的钩子函数赋值1
			export -n "PI_RAN_$func=1"
            # 运行不加头的钩子函数
			$func "$1" "$2"
		}
	done
}

pivot() { # <new_root> <old_root>
	/bin/mount -o noatime,move /proc $1/proc && \
	pivot_root $1 $1$2 && {
		/bin/mount -o noatime,move $2/dev /dev
		/bin/mount -o noatime,move $2/tmp /tmp
		/bin/mount -o noatime,move $2/sys /sys 2>&-
		/bin/mount -o noatime,move $2/overlay /overlay 2>&-
		return 0
	}
}

fopivot() { # <rw_root> <ro_root> <dupe?>
	/bin/mount -o noatime,lowerdir=/,upperdir=$1 -t overlayfs "overlayfs:$1" /mnt
	pivot /mnt $2
}

ramoverlay() {
	mkdir -p /tmp/root
	/bin/mount -t tmpfs -o noatime,mode=0755 root /tmp/root
	fopivot /tmp/root /rom 1
}
