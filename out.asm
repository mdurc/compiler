.data
newline: .asciiz "\n"
L0: .word 15
L1: .word 1
L2: .word 0
L3: .word 0
L9: .asciiz "Fizz"
L12: .asciiz "Buzz"
L15: .asciiz "Buzz\n"

.text
.align 2
.globl main


main:
addi $sp, $sp, -4
sw $ra, 0($sp)
# function content

# == while-conditional start ==
L4:
la $t1, L1
lw $t0, 0($t1)
la $t2, L0
lw $t1, 0($t2)
sle $v0, $t0, $t1
beq $v0, $zero, L5
L6:
la $t0, L2
lw $t1, 0($t0)
la $t2, L1
lw $t2, 0($t2)
li $t3, 3
div $t2, $t3
mfhi $t1
sw $t1, 0($t0)
la $t0, L3
lw $t1, 0($t0)
la $t2, L1
lw $t2, 0($t2)
li $t3, 5
div $t2, $t3
mfhi $t1
sw $t1, 0($t0)
# == if-conditional start ==
la $t1, L2
lw $t0, 0($t1)
li $t1, 0
seq $v0, $t0, $t1
beq $v0, $zero, L7
L8:
# == print start ==
la $a0, L9
li $v0, 4
syscall
# == print end ==
# == if-conditional start ==
la $t1, L3
lw $t0, 0($t1)
li $t1, 0
seq $v0, $t0, $t1
beq $v0, $zero, L10
L11:
# == print start ==
la $a0, L12
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L10:
# == print start ==
la $a0, newline
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L7:
# == if-conditional start ==
la $t1, L2
lw $t0, 0($t1)
li $t1, 0
sne $v0, $t0, $t1
beq $v0, $zero, L13
la $t1, L3
lw $t0, 0($t1)
li $t1, 0
seq $v0, $t0, $t1
beq $v0, $zero, L13
L14:
# == print start ==
la $a0, L15
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L13:
# == if-conditional start ==
la $t1, L2
lw $t0, 0($t1)
li $t1, 0
sne $v0, $t0, $t1
beq $v0, $zero, L16
la $t1, L3
lw $t0, 0($t1)
li $t1, 0
sne $v0, $t0, $t1
beq $v0, $zero, L16
L17:
# == print start ==
la $t0, L1
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L16:
la $t0, L1
lw $t1, 0($t0)
li $t2, 1
add $t1, $t1, $t2
sw $t1, 0($t0)
j L4
# == while-conditional loop/end ==
L5:
# returning here
li $v0, 0
# unloading function
lw $ra, 0($sp)
addi $sp, $sp, 4
jr $ra

# ====== START STRCMP ======
strcmp:
move $t0, $zero
move $t1, $a0
move $t2, $a1
strcmp_loop:
# read current byte
lb $t3, 0($t1)
lb $t4, 0($t2)
beq $t3, $zero, strcmp_checklower  # s1 is finished
beq $t4, $zero, strcmp_higher      #str2 is finished
blt $t3, $t4, strcmp_lower         #if str1 < str2 -> str1 is strcmp_lower
bgt $t3, $t4, strcmp_higher        #if str1 > str2 -> str1 is strcmp_higher
# move to next bytes
addi $t1, $t1,1
addi $t2, $t2,1
j strcmp_loop

strcmp_checklower: # if str2 is not finished str1 < str2
beq $t4, $zero, strcmp_equal
j strcmp_lower

strcmp_equal:
li $v0, 1
jr $ra

strcmp_lower:
li $v0, 0
jr $ra

strcmp_higher:
li $v0, 0
jr $ra
# ====== END STRCMP ======
