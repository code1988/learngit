/*
 * wpa_supplicant/hostapd - State machine definitions
 * Copyright (c) 2002-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This file includes a set of pre-processor macros that can be used to
 * implement a state machine. In addition to including this header file, each
 * file implementing a state machine must define STATE_MACHINE_DATA to be the
 * data structure including state variables (enum machine_state,
 * Boolean changed), and STATE_MACHINE_DEBUG_PREFIX to be a string that is used
 * as a prefix for all debug messages. If SM_ENTRY_MA macro is used to define
 * a group of state machines with shared data structure, STATE_MACHINE_ADDR
 * needs to be defined to point to the MAC address used in debug output.
 * SM_ENTRY_M macro can be used to define similar group of state machines
 * without this additional debug info.
 * 本文件包含了一系列宏，这些宏被用于实现一个状态机。
 * 具体每个文件在实现一个状态机时，除了要包含本头文件，还必须定义以下这些宏：
 * STATE_MACHINE_DATA           - 状态机的管理块
 *
 * 以下这些宏是可选的：
 * STATE_MACHINE_DEBUG_PREFIX   - 调试信息的前缀
 * SM_ENTRY_MA                  - 类似于SM_ENTRY,只是它用于共享管理块的状态机组(eapol_auth_sm.c中就使用了多个状态机共享一个总的管理块这种架构) 
 * STATE_MACHINE_ADDR           - 指向MAC地址（用于调试时输出）
 * SM_ENTRY_M                   - 类似于SM_ENTRY_MA,只是没有这些附加调试信息
 *
 *
 * 状态机调用流程：
 *      SM_STEP_RUN -> SM_ENTER_GLOBAL -> SM_ENTRY_MA 
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

/**
 * SM_STATE - Declaration of a state machine function
 * @machine: State machine name
 * @state: State machine state
 * 定义了一个状态机具体状态要进行的动作
 *
 * This macro is used to declare a state machine function. It is used in place
 * of a C function definition to declare functions to be run when the state is
 * entered by calling SM_ENTER or SM_ENTER_GLOBAL.
 * 该宏定义的函数通常是被SM_ENTER/SM_ENTER_GLOBAL调用的
 */
#define SM_STATE(machine, state) \
static void sm_ ## machine ## _ ## state ## _Enter(STATE_MACHINE_DATA *sm, \
	int global)

/**
 * SM_ENTRY - State machine function entry point
 * @machine: State machine name
 * @state: State machine state
 *
 * This macro is used inside each state machine function declared with
 * SM_STATE. SM_ENTRY should be in the beginning of the function body, but
 * after declaration of possible local variables. This macro prints debug
 * information about state transition and update the state machine state.
 * 状态机进入某个状态需要执行的动作,通常就是设置新的状态
 *
 * 备注：该宏只能用于SM_STATE定义的函数内部，并且最好放在开头
 *       该宏主要工作是对状态机状态进行了update
 */
#define SM_ENTRY(machine, state) \
if (!global || sm->machine ## _state != machine ## _ ## state) { \
	sm->changed = TRUE; \
	wpa_printf(MSG_DEBUG, STATE_MACHINE_DEBUG_PREFIX ": " #machine \
		   " entering state " #state); \
} \
sm->machine ## _state = machine ## _ ## state;

/**
 * SM_ENTRY_M - State machine function entry point for state machine group
 * @machine: State machine name
 * @_state: State machine state
 * @data: State variable prefix (full variable: prefix_state)
 *
 * This macro is like SM_ENTRY, but for state machine groups that use a shared
 * data structure for more than one state machine. Both machine and prefix
 * parameters are set to "sub-state machine" name. prefix is used to allow more
 * than one state variable to be stored in the same data structure.
 * 类似于SM_ENTRY,只是此宏用于状态机组
 */
#define SM_ENTRY_M(machine, _state, data) \
if (!global || sm->data ## _ ## state != machine ## _ ## _state) { \
	sm->changed = TRUE; \
	wpa_printf(MSG_DEBUG, STATE_MACHINE_DEBUG_PREFIX ": " \
		   #machine " entering state " #_state); \
} \
sm->data ## _ ## state = machine ## _ ## _state;

/**
 * SM_ENTRY_MA - State machine function entry point for state machine group
 * @machine: State machine name
 * @_state: State machine state
 * @data: State variable prefix (full variable: prefix_state)
 *
 * This macro is like SM_ENTRY_M, but a MAC address is included in debug
 * output. STATE_MACHINE_ADDR has to be defined to point to the MAC address to
 * be included in debug.
 * 类似于SM_ENTRY_M,只是此宏在调试输出时会附加一个MAC地址打印
 */
#define SM_ENTRY_MA(machine, _state, data) \
if (!global || sm->data ## _ ## state != machine ## _ ## _state) { \
	sm->changed = TRUE; \
	wpa_printf(MSG_DEBUG, STATE_MACHINE_DEBUG_PREFIX ": " MACSTR " " \
		   #machine " entering state " #_state, \
		   MAC2STR(STATE_MACHINE_ADDR)); \
} \
sm->data ## _ ## state = machine ## _ ## _state;

/**
 * SM_ENTER - Enter a new state machine state
 * @machine: State machine name
 * @state: State machine state
 * 状态机进入一个新的状态
 *
 * This macro expands to a function call to a state machine function defined
 * with SM_STATE macro. SM_ENTER is used in a state machine step function to
 * move the state machine to a new state.
 * 本宏用于SM_STEP定义的函数内
 * 本宏展开后就是调用了一个具体SM_STATE定义的函数
 */
#define SM_ENTER(machine, state) \
sm_ ## machine ## _ ## state ## _Enter(sm, 0)

/**
 * SM_ENTER_GLOBAL - Enter a new state machine state based on global rule
 * @machine: State machine name
 * @state: State machine state
 * 状态机进入一个新的状态(附加一个全局的规则,通常是UCT = 1,表示无条件转移)
 *
 * This macro is like SM_ENTER, but this is used when entering a new state
 * based on a global (not specific to any particular state) rule. A separate
 * macro is used to avoid unwanted debug message floods when the same global
 * rule is forcing a state machine to remain in on state.
 */
#define SM_ENTER_GLOBAL(machine, state) \
sm_ ## machine ## _ ## state ## _Enter(sm, 1)

/**
 * SM_STEP - Declaration of a state machine step function
 * @machine: State machine name
 *
 * This macro is used to declare a state machine step function. It is used in
 * place of a C function definition to declare a function that is used to move
 * state machine to a new state based on state variables. This function uses
 * SM_ENTER and SM_ENTER_GLOBAL macros to enter new state.
 * 定义一个函数用于检查状态机的条件变量，然后根据情况进行状态切换
 * 函数在具体实现时又是通过调用SM_ENTER/SM_ENTER_GLOBAL宏进入一个新的状态的
 */
#define SM_STEP(machine) \
static void sm_ ## machine ## _Step(STATE_MACHINE_DATA *sm)

/**
 * SM_STEP_RUN - Call the state machine step function
 * @machine: State machine name
 *
 * This macro expands to a function call to a state machine step function
 * defined with SM_STEP macro.
 * 调用使用SM_STEP定义的状态机循环和分支判断函数
 */
#define SM_STEP_RUN(machine) sm_ ## machine ## _Step(sm)

#endif /* STATE_MACHINE_H */
