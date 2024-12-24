.data
x: .asciiz "test var1"
y: .word 15
z: .asciiz "i am var3"
L0: .asciiz "Testing string variable x: "
L1: .asciiz "\nTesting int variable y: "
L2: .asciiz "\nTesting string variable z: "
L3: .asciiz "\n"

.text
.align 2
.globl main


main:
addi $sp, $sp, -4
sw $ra, 0($sp)
# function content

la $a0, L0
li $v0, 4
syscall

la $a0, x
li $v0, 4
syscall

la $a0, L1
li $v0, 4
syscall

la $t0, y
lw $a0, 0($t0)
li $v0, 1
syscall

la $a0, L2
li $v0, 4
syscall

la $a0, z
li $v0, 4
syscall

la $a0, L3
li $v0, 4
syscall

# returning here
li $v0, 0
# unloading function
lw $ra, 0($sp)
addi $sp, $sp, 4
jr $ra
