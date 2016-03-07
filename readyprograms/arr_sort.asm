; Example program. It sorts an array

; iLab, 2016

;-------------------------------------------------------------------------------
; Code 
;-------------------------------------------------------------------------------
; Program entry point

_start:
    
    ; set massive pointer
    ldi     r1, ARR_SIZE
    muli    r1, 2
    addi    r1, array
    subi    r1, 2
    
_loop1:
    ldi     r0, array
_loop2:
    pushall
    call set_10_regs
    popall
    call bubble_op
    cmp r0, r1
    jl _loop2
    subi r1, 2
    cmpi r1, array
    jge _loop1

    call set_10_regs

; MUST HAVE:
;   [r0-r9]: 1 2 3 4 5 6 7 8 9 11

final_loop:
    jmp     final_loop


; checks [r0] > [r0 + 4], arr_pointer++

bubble_op:
    ; save regs
    push r1
    push r2

    ldm r1, r0
    addi r0, 2
    ldm r2, r0

    cmp r1, r2
    jle bubble_op_skip
    push r3
    mov r3, r1
    mov r1, r2
    mov r2, r3 
    pop r3
    subi r0, 2
    stm r1, r0
    addi r0, 2
    stm r2, r0

bubble_op_skip:    
    ; restore regs
    pop r2
    pop r1
    ret

; store array values to r(0)-r(10) to see result in simics through chip0->gprs

set_10_regs:
    ldi r0, array
    addi r0, 2
    ldm r1, r0
    addi r0, 2
    ldm r2, r0
    addi r0, 2
    ldm r3, r0
    addi r0, 2
    ldm r4, r0
    addi r0, 2
    ldm r5, r0
    addi r0, 2
    ldm r6, r0
    addi r0, 2
    ldm r7, r0
    addi r0, 2
    ldm r8, r0
    addi r0, 2
    ldm r9, r0
    ldm r0, array
    ret



;-------------------------------------------------------------------------------
; Data
;-------------------------------------------------------------------------------

array:      dw      1 4 7 2 8 6 5 9 3 11
ARR_SIZE    equ     10
