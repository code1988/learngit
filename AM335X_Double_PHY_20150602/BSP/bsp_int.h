/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*                                         INTERRUPT CONTROLLER
*
*                                      Texas Instruments AM3517
*
*                                              on the
*
*                                             EVM-AM3517
*                                         Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : FF
*                 JM
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               BSP_INT present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  BSP_INT_PRESENT
#define  BSP_INT_PRESENT


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_INT_MODULE
#define  BSP_INT_EXT
#else
#define  BSP_INT_EXT  extern
#endif


/*
*********************************************************************************************************
*                                        DEFAULT CONFIGURATION
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/
/* ------------ INTERRUPT PRIORITY DEFINES ---------- */
#define  BSP_INT_PRIO_LEVEL_MASK             DEF_BIT_MASK(7, 0)
#define  BSP_INT_PRIO_LEVEL_00               0u
#define  BSP_INT_PRIO_LEVEL_01               1u
#define  BSP_INT_PRIO_LEVEL_02               2u
#define  BSP_INT_PRIO_LEVEL_03               3u
#define  BSP_INT_PRIO_LEVEL_04               4u
#define  BSP_INT_PRIO_LEVEL_05               5u
#define  BSP_INT_PRIO_LEVEL_06               6u
#define  BSP_INT_PRIO_LEVEL_07               7u

#define  BSP_INT_MASK_SRC_OFFSET_00          0x80u


/*
*********************************************************************************************************
*                                               INT DEFINES
*********************************************************************************************************
*/
//删除这了的中断定义，定义在库文件interrupt.h


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         BSP_IntClr           (CPU_INT08U     int_id);
void         BSP_IntDis           (CPU_INT08U     int_id);
void         BSP_IntDisAll        (void);
void         BSP_IntEn            (CPU_INT08U     int_id);
void         BSP_IntInit          (void);
void         BSP_IntVectReg       (CPU_INT08U int_id, 
                                    void (*fnHandler)(void) );
void         BSP_IntHandler       (CPU_INT32U     src_nbr);


void  BSP_IntVectReg (CPU_INT08U int_id, void (*fnHandler)(void) );

#endif                                                          /* End of module include.                               */
