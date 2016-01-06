;
;********************************************************************************************************
;                                    EXCEPTION VECTORS & STARTUP CODE
;
; File      : cstartup.s
; For       : ARM7 or ARM9
; Toolchain : IAR EWARM V5.10 and higher
;********************************************************************************************************
;

    MODULE  ?cstartup

;********************************************************************************************************
;                                           MACROS AND DEFINIITIONS
;********************************************************************************************************

                                ; Mode, correspords to bits 0-5 in CPSR
MODE_BITS	DEFINE	0x1F		; Bit mask for mode bits in CPSR
USR_MODE	DEFINE	0x10		; User mode
FIQ_MODE	DEFINE	0x11		; Fast Interrupt Request mode
IRQ_MODE	DEFINE	0x12		; Interrupt Request mode
SVC_MODE	DEFINE	0x13		; Supervisor mode
ABT_MODE	DEFINE	0x17		; Abort mode
UND_MODE	DEFINE	0x1B		; Undefined Instruction mode
SYS_MODE	DEFINE	0x1F		; System mode

CP_DIS_MASK DEFINE  0xFFFFEFFA


;********************************************************************************************************
;                                            ARM EXCEPTION VECTORS
;********************************************************************************************************

    SECTION .intvec:CODE:NOROOT(2)
    PUBLIC  __vector
    PUBLIC  __iar_program_start

    IMPORT  OS_CPU_ARM_ExceptUndefInstrHndlr
    IMPORT  OS_CPU_ARM_ExceptSwiHndlr
    IMPORT  OS_CPU_ARM_ExceptPrefetchAbortHndlr
    IMPORT  OS_CPU_ARM_ExceptDataAbortHndlr
    IMPORT  OS_CPU_ARM_ExceptIrqHndlr
    IMPORT  OS_CPU_ARM_ExceptFiqHndlr

    ARM

__vector:

    LDR     PC,Reset_Addr           ; Reset
    LDR     PC,Undefined_Addr       ; Undefined instructions
    LDR     PC,SWI_Addr             ; Software interrupt (SWI/SVC)
    LDR     PC,Prefetch_Addr        ; Prefetch abort
    LDR     PC,Abort_Addr           ; Data abort
    LDR     PC,Unused_Addr          ; RESERVED
    LDR     PC,IRQ_Addr             ; IRQ
    LDR     PC,FIQ_Addr             ; FIQ

    DATA

Reset_Addr:     DCD   __iar_program_start

Undefined_Addr: DCD   OS_CPU_ARM_ExceptUndefInstrHndlr
SWI_Addr:       DCD   OS_CPU_ARM_ExceptSwiHndlr
Prefetch_Addr:  DCD   OS_CPU_ARM_ExceptPrefetchAbortHndlr
Abort_Addr:     DCD   OS_CPU_ARM_ExceptDataAbortHndlr
Unused_Addr:    DCD   .
IRQ_Addr:       DCD   OS_CPU_ARM_ExceptIrqHndlr
FIQ_Addr:       DCD   OS_CPU_ARM_ExceptFiqHndlr


;********************************************************************************************************
;                                   LOW-LEVEL INITIALIZATION
;********************************************************************************************************

    SECTION FIQ_STACK:DATA:NOROOT(3)
    SECTION IRQ_STACK:DATA:NOROOT(3)
    SECTION SVC_STACK:DATA:NOROOT(3)
    SECTION ABT_STACK:DATA:NOROOT(3)
    SECTION UND_STACK:DATA:NOROOT(3)
    SECTION CSTACK:DATA:NOROOT(3)
    SECTION text:CODE:NOROOT(2)
    REQUIRE __vector
    EXTERN  __cmain
    EXTWEAK __iar_init_core
    EXTWEAK __iar_init_vfp
    PUBLIC  __iar_program_start
    EXTERN  lowlevel_init

    ARM
__iar_program_start:
?cstartup:

                                                ; Add initialization needed before ...
                                                ; ... setup of stackpointers here.
                                                ; Disable Addr translation, D cache ...
                                                ; ... and enable I cache
    MRC         p15,0,R1,C1,C0,0
    LDR         R0,=CP_DIS_MASK      ;; 0xFFFFEFFA
    AND         R1,R1,R0
    ORR         R1,R1,#(1<<12)
    MCR         p15,0,R1,C1,C0,0


;********************************************************************************************************
;                                    STACK POINTER INITIALIZATION
;********************************************************************************************************

        mrs     r0,cpsr                             ; Original PSR value
        bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
        orr     r0,r0,#SVC_MODE                     ; Set Supervisor mode bits
        msr     cpsr_c,r0                           ; Change the mode
        ldr     sp,=SFE(SVC_STACK)                  ; End of SVC_STACK

        bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
        orr     r0,r0,#ABT_MODE                     ; Set Abort mode bits
        msr     cpsr_c,r0                           ; Change the mode
        ldr     sp,=SFE(ABT_STACK)                  ; End of ABT_STACK

        bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
        orr     r0,r0,#UND_MODE                     ; Set Undefined mode bits
        msr     cpsr_c,r0                           ; Change the mode
        ldr     sp,=SFE(UND_STACK)                  ; End of UND_STACK

        bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
        orr     r0,r0,#FIQ_MODE                     ; Set FIR mode bits
        msr     cpsr_c,r0                           ; Change the mode
        ldr     sp,=SFE(FIQ_STACK)                  ; End of FIQ_STACK

        bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
        orr     r0,r0,#IRQ_MODE                     ; Set IRQ mode bits
        msr     cpsr_c,r0                           ; Change the mode
        ldr     sp,=SFE(IRQ_STACK)                  ; End of IRQ_STACK

        bic     r0,r0,#MODE_BITS                    ; Clear the mode bits
        orr     r0,r0,#SYS_MODE                     ; Set System mode bits
        msr     cpsr_c,r0                           ; Change the mode
        ldr     sp,=SFE(CSTACK)                     ; End of CSTACK


        ldr     r0,=SFB(.intvec)
        mcr     p15, 0, r0, c12, c0, 0



        ;; Turn on core features assumed to be enabled.
        BL      __iar_init_core

        ;; Initialize VFP (if needed).
        BL      __iar_init_vfp


; Add more initialization here

; Continue to ?main for more IAR specific system startup
        B       __cmain

                END
