.data
x: .word 32
L1: .asciiz "Valid\n"
L3: .asciiz "Valid\n"
L5: .asciiz "Invalid\n"
L7: .asciiz "Valid\n"
y: .word 15
L9: .asciiz "Invalid\n"
L11: .asciiz "Invalid\n"
L13: .asciiz "Valid !=\n"
L15: .asciiz "Valid <\n"
L17: .asciiz "Valid <=\n"
z: .word 15
L19: .asciiz "Valid\n"
L21: .asciiz "Valid\n"
a: .asciiz "test"
g: .asciiz "test"
L23: .asciiz "Valid\n"
L25: .asciiz "Valid\n"
c: .asciiz "wrong"
L27: .asciiz "Invalid\n"
L29: .asciiz "Invalid\n"

.text
.align 2
.globl main


main:
addi $sp, $sp, -4
sw $ra, 0($sp)
# function content

li $t0, 3
li $t1, 3
seq $t0, $t0, $t1
beq $t0, $zero, L0

la $a0, L1
li $v0, 4
syscall

L0:
la $t1, x
lw $t0, 0($t1)
li $t1, 32
seq $t0, $t0, $t1
beq $t0, $zero, L2

la $a0, L3
li $v0, 4
syscall

L2:
la $t1, x
lw $t0, 0($t1)
li $t1, 3
seq $t0, $t0, $t1
beq $t0, $zero, L4

la $a0, L5
li $v0, 4
syscall

L4:
li $t0, 32
la $t2, x
lw $t1, 0($t2)
seq $t0, $t0, $t1
beq $t0, $zero, L6

la $a0, L7
li $v0, 4
syscall

L6:
la $t1, x
lw $t0, 0($t1)
la $t2, y
lw $t1, 0($t2)
seq $t0, $t0, $t1
beq $t0, $zero, L8

la $a0, L9
li $v0, 4
syscall

L8:
la $t1, y
lw $t0, 0($t1)
la $t2, x
lw $t1, 0($t2)
seq $t0, $t0, $t1
beq $t0, $zero, L10

la $a0, L11
li $v0, 4
syscall

L10:
la $t1, x
lw $t0, 0($t1)
la $t2, y
lw $t1, 0($t2)
sne $t0, $t0, $t1
beq $t0, $zero, L12

la $a0, L13
li $v0, 4
syscall

L12:
la $t1, y
lw $t0, 0($t1)
la $t2, x
lw $t1, 0($t2)
slt $t0, $t0, $t1
beq $t0, $zero, L14

la $a0, L15
li $v0, 4
syscall

L14:
la $t1, x
lw $t0, 0($t1)
la $t2, x
lw $t1, 0($t2)
sle $t0, $t0, $t1
beq $t0, $zero, L16

la $a0, L17
li $v0, 4
syscall

L16:
la $t1, y
lw $t0, 0($t1)
la $t2, z
lw $t1, 0($t2)
seq $t0, $t0, $t1
beq $t0, $zero, L18

la $a0, L19
li $v0, 4
syscall

L18:
la $t1, z
lw $t0, 0($t1)
la $t2, y
lw $t1, 0($t2)
seq $t0, $t0, $t1
beq $t0, $zero, L20

la $a0, L21
li $v0, 4
syscall

L20:
li $t0, 1
li $t1, 1
seq $t0, $t0, $t1
beq $t0, $zero, L22

la $a0, L23
li $v0, 4
syscall

L22:
li $t0, 1
li $t1, 1
seq $t0, $t0, $t1
beq $t0, $zero, L24

la $a0, L25
li $v0, 4
syscall

L24:
li $t0, 1
li $t1, 0
seq $t0, $t0, $t1
beq $t0, $zero, L26

la $a0, L27
li $v0, 4
syscall

L26:
li $t0, 1
li $t1, 0
seq $t0, $t0, $t1
beq $t0, $zero, L28

la $a0, L29
li $v0, 4
syscall

L28:
# returning here
li $v0, 0
# unloading function
lw $ra, 0($sp)
addi $sp, $sp, 4
jr $ra
