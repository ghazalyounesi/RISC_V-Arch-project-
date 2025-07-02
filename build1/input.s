    .section .data
    .org 0x100

val1:   .word 10
val2:   .word 3
val3:   .word -20
arr:    .byte 0x01, 0x02, 0x03, 0x04

    .section .text
    .org 0x1000
    .globl _start
_start:

    # ==== Type U ====
    lui x1, 0x12345           # x1 = 0x12345000
    auipc x2, 0x00001         # x2 = PC + 0x1000

    # ==== Type J ====
    jal x3, label_jump        # x3 = return addr

continue:

    # ==== Type R ====
    add x4, x1, x2
    sub x5, x4, x2
    xor x6, x4, x5
    or  x7, x5, x6
    and x8, x6, x7
    sll x9, x4, x5
    srl x10, x7, x6
    sra x11, x7, x6
    slt x12, x5, x6
    sltu x13, x6, x5

    # ==== Multiply Extension (R Type) ====
    mul x14, x1, x2
    mulh x15, x1, x2
    mulhsu x16, x1, x2
    mulhu x17, x1, x2
    div x18, x1, x2
    divu x19, x1, x2
    rem x20, x1, x2
    remu x21, x1, x2

    # ==== Type I ====
    li x22, 100
    ori x23, x22, 0xFF
    andi x24, x23, 0x0F
    slli x25, x24, 2
    srli x26, x25, 1

    # ==== Load (I2-type) ====
    # آدرس val1 = 0x100
    lui x27, 0x0          # upper 20 bits = 0
    addi x27, x27, 0x100  # x27 = 0x100

    lb  x28, 0(x27)
    lh  x29, 0(x27)
    lw  x30, 0(x27)
    lbu x31, 0(x27)
    lhu x3, 0(x27)

    # ==== JALR ====
    li x5, 4
    jalr x6, 0(x5)

    # ==== Store (S Type) ====
    # val2 در آدرس 0x104 هست
    lui x7, 0x0
    addi x7, x7, 0x104

    sb x1, 0(x7)
    sh x2, 0(x7)
    sw x3, 0(x7)

    # ==== Branches (Type B) ====
    beq x1, x1, equal_label
    bne x1, x2, notequal_label
    blt x3, x4, lt_label
    bge x4, x3, ge_label
    bltu x5, x6, ltu_label
    bgeu x6, x5, geu_label

equal_label:
    li x10, 1

notequal_label:
    li x11, 2

lt_label:
    li x12, 3

ge_label:
    li x13, 4

ltu_label:
    li x14, 5

geu_label:
    li x15, 6

label_jump:
    nop
    jal x0, continue
