# single cycle - 241 cycles
# pipelined (no branch prediction, compensate for stalls) - 284 cycles 
# assuming pipelined cycle is ~4 times faster than a single cycle, we get 3.4 times speedup

addi x15,x0,20
LOOP:
    add x5, x1, x2
    sub x6, x3, x4
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    addi x1, x1, 1
    bne x1, x15, LOOP