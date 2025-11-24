# pipelined with fwd and hz

######### static branch predictor is better than no branch predictor #########
# without prediction : 139
# with prediction : 125


addi x31,x31,10

loop:
    addi x1,x1,1
    addi x2,x2,2
    addi x3,x3,3
    addi x4,x4,4
    addi x5,x5,5
    addi x6,x6,6
    addi x7,x7,7
    addi x8,x8,8
    addi x9,x9,9

    addi x31,x31,-1
    bne x0,x31,loop

nop
nop
nop
nop
nop


######### dynamic branch predictor is better than static branch predictor #########
# static prediction : 165
# dynamic prediction : 146


addi x31,x31,10
addi x30,x0,1

loop:
    addi x1,x1,1
    addi x2,x2,2
    addi x3,x3,3
    addi x4,x4,4
    addi x5,x5,5

    if:
        beq x0,x1,loop # 32

    addi x6,x6,6
    addi x7,x7,7
    addi x8,x8,8
    addi x9,x9,9

    addi x31,x31,-1
    bne x0,x31,loop # 52

nop
nop
nop
nop
nop



######### dynamic branch predictor is bad #########
# no prediction : 241
# static prediction : 229
# dynamic prediction : 245


addi x31,x31,10
addi x29,x0,2

loop:
    addi x1,x1,1
    addi x2,x2,2
    addi x3,x3,3
    addi x4,x4,4
    addi x5,x5,5

    rem x30,x1,x29
    if:
        beq x0,x30,loop

    addi x6,x6,6
    addi x7,x7,7
    addi x8,x8,8
    addi x9,x9,9

    addi x31,x31,-1
    bne x0,x31,loop

nop
nop
nop
nop
nop
