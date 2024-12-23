.data
    newline: .asciiz "\n"
.text
    .align 2
    .globl main
main:
    addi $sp, $sp, -4
    sw $ra, 4($sp)
THIS IS A TEST
    lw $ra, 4($sp)
    addi $sp, $sp, 4
    jr $ra
