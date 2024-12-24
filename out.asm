.data
L0: .asciiz "\nhi there\n"
L1: .asciiz ": is the answer\n"
L2: .asciiz "this is a new print line\n"
.text
.align 2
.globl main


main:
addi $sp, $sp, -4
sw $ra, 0($sp)
# function content

li $a0, 32
li $v0, 1
syscall
la $a0, L0
li $v0, 4
syscall
li $a0, 15
li $v0, 1
syscall
la $a0, L1
li $v0, 4
syscall
la $a0, L2
li $v0, 4
syscall

# returning here
li $v0, 0
# unloading function
lw $ra, 0($sp)
addi $sp, $sp, 4
jr $ra
