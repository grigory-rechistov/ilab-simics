; Example programm. It finds the summ of a massive

; iLab, 2016

;-------------------------------------------------------------------------------
; Code 
;-------------------------------------------------------------------------------
; Program entry point

_start:
    
    ; set massive pointer
    ldi     r0, massive

    ; clear register with summ
    xor     r1, r1

    ; clear test value register
    xor     r2, r2

    ; get first value
    ldm     r2, r0

    cmpi    r2, 0
    jz      final_loop
    

summ_loop:

    add     r1, r2
                
    addi    r0, 2
    
    ldm     r2, r0

    cmpi    r2, 0

    jnz     summ_loop


; MUST HAVE:
;    r1 == 55

final_loop:
    jmp     final_loop

;-------------------------------------------------------------------------------
; Data
;-------------------------------------------------------------------------------

massive:    dw 1 2 3 4 5 6 7 8 9 10 0
