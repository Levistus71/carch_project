.text

# data hazard immediately
addi x3,x0,3
addi x4,x3,3
nop
nop
nop
nop

# data hazard after 1 instruction
addi x3,x0,3
nop
addi x4,x3,3
nop
nop
nop
nop


# (no) data hazard after 2 instructions
addi x3,x0,3
nop
nop
addi x4,x3,3
nop
nop
nop
nop


# load use hazard
ld x3,0(x0)
addi x4,x3,3
nop
nop
nop
nop


# load use hazard after 1 instruction
ld x3,0(x0)
nop
addi x4,x3,3
nop
nop
nop
nop

# (no) load use hazard after 2 instructions
ld x3,0(x0)
nop
nop
addi x4,x3,3
nop
nop
nop
nop


# control hazard
beq x0,x0,Label
addi x3,x0,123
addi x4,x0,213
addi x5,x0,231
addi x6,x0,132
Label:
    addi x27,x0,123
    addi x28,x0,132
    addi x29,x0,342
    addi x30,x0,512
nop
nop
nop
nop
