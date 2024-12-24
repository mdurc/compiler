.data
L0: .asciiz "test\n"
L1: .asciiz "new\n"
.text
.align 2
.globl main


main:
addi $sp, $sp, -4
sw $ra, 0($sp)
# function content

li $v0, 4
la $a0, L0
syscall
la $a0, L1
syscall

# returning here
li $v0, 0
# unloading function
lw $ra, 0($sp)
addi $sp, $sp, 4
jr $ra
