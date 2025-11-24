# pipelined -> 156
# dual issue -> 118

addi x31,x31,5
addi x30,x30,1000
addi x30,x30,1000

loop:
    lw x20,0(x0)
    addi x1,x1,1

    lw x21,0(x0)
    addi x2,x2,2

    lw x22,0(x0)
    addi x3,x3,3

    lw x23,0(x0)
    addi x4,x4,4

    lw x24,0(x0)
    addi x5,x5,5

    lw x25,0(x0)
    addi x6,x6,6

    lw x26,0(x0)
    add x7,x7,x20

    sw x1,0(x30)
    add x8,x8,x21

    sw x2,8(x30)
    add x9,x9,x22

    sw x3,16(x30)
    add x10,x10,x23

    sw x4,24(x30)
    add x10,x10,x24

    sw x5,32(x30)
    add x11,x11,x25

    sw x6,32(x30)
    add x12,x12,x26

    addi x31,x31,-1
    bne x0,x31,loop