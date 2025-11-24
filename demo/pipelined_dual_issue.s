# no branch predictor to be fair
# pieplined with forwarding and hazard detection

######
# pipelined -> 159
# dual issue -> 196

addi x31,x0,5

loop:
    lw x2,0(x0)
    add x3,x0,x2
    add x4,x3,x3
    add x5,x3,x4
    lw x2,0(x0)
    add x6,x5,x3
    sub x2,x3,x5
    addi x7,x2,123
    lw x2,0(x0)
    add x3,x0,x2
    add x4,x3,x3
    add x5,x3,x4
    lw x2,0(x0)
    add x6,x5,x3
    sub x2,x3,x5
    addi x7,x2,123
    lw x2,0(x0)
    add x3,x0,x2
    add x4,x3,x3
    add x5,x3,x4
    lw x2,0(x0)
    add x6,x5,x3
    sub x2,x3,x5
    addi x7,x2,123

    addi x31,x31,-1
    bne x0,x31,loop


    
#####
# pipelined -> 94
# dual issue -> 186


addi  x31, x0, 5
loop:
    lw x2,0(x0)
    add x3,x0,x2
    add x4,x3,x3
    add x5,x3,x4
    add x6,x5,x3
    sub x2,x3,x5
    addi x7,x2,123

    add x3,x0,x2
    add x4,x3,x3
    add x5,x3,x4
    add x6,x5,x3
    sub x2,x3,x5
    addi x7,x2,123

    addi x31,x31,-1
    bne x0,x31,loop