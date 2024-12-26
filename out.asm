.data
newline: .asciiz "\n"
L0: .word 0
L1: .word 15
L8: .asciiz "Is i==32? "
L11: .asciiz "... YES!\n"
L12: .word 1
L13: .asciiz "/"
L14: .asciiz " ==> "
L15: .asciiz "*"
L16: .asciiz " ==> "
L17: .word 15
L18: .word 0
L19: .asciiz "%"
L20: .asciiz " ==> "
L21: .asciiz "%"
L22: .asciiz " ==> "
L23: .asciiz "%"
L24: .asciiz " ==> "
L25: .word 23523
L26: .asciiz "Using bit-masking &: "
L27: .asciiz "%"
L28: .asciiz " ==> "
L29: .asciiz "%"
L30: .asciiz " ==> "
L33: .asciiz "str"
L34: .asciiz "str"
L35: .asciiz "String compare-1 works\n"
L38: .asciiz "st"
L39: .asciiz "str"
L40: .asciiz "String compare-2 works\n"
L43: .asciiz "st"
L44: .asciiz "str"
L45: .asciiz "String compare-3 works\n"
L48: .asciiz "str"
L49: .asciiz "str"
L50: .asciiz "String compare-4 works\n"
L53: .asciiz "string"
L54: .asciiz "str"
L55: .asciiz "String compare-5 works\n"
L58: .asciiz "str"
L59: .asciiz "str"
L60: .asciiz "WRONG\n"

.text
.align 2
.globl main


main:
addi $sp, $sp, -4
sw $ra, 0($sp)
# function content

# == while-conditional start ==
L2:
la $t1, L0
lw $t0, 0($t1)
la $t2, L1
lw $t1, 0($t2)
sle $v0, $t0, $t1
beq $v0, $zero, L3
L4:
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L0
lw $t1, 0($t0)
li $t2, 1
add $t1, $t1, $t2
sw $t1, 0($t0)
j L2
# == while-conditional loop/end ==
L3:
# == print start ==
la $a0, newline
li $v0, 4
syscall
# == print end ==
# == while-conditional start ==
L5:
la $t1, L0
lw $t0, 0($t1)
li $t1, 0
sge $v0, $t0, $t1
beq $v0, $zero, L6
L7:
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L0
lw $t1, 0($t0)
li $t2, 2
sub $t1, $t1, $t2
sw $t1, 0($t0)
j L5
# == while-conditional loop/end ==
L6:
la $t0, L0
lw $t1, 0($t0)
li $t2, 8
li $t3, 24
add $t1, $t2, $t3
sw $t1, 0($t0)
# == print start ==
la $a0, L8
li $v0, 4
syscall
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
# == print end ==
# == if-conditional start ==
la $t1, L0
lw $t0, 0($t1)
li $t1, 32
seq $v0, $t0, $t1
beq $v0, $zero, L9
L10:
# == print start ==
la $a0, L11
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L9:
la $t0, L0
lw $t1, 0($t0)
la $t2, L1
lw $t2, 0($t2)
la $t3, L1
lw $t3, 0($t3)
add $t1, $t2, $t3
sw $t1, 0($t0)
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L0
lw $t1, 0($t0)
la $t2, L1
lw $t2, 0($t2)
li $t3, 3
add $t1, $t2, $t3
sw $t1, 0($t0)
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L0
lw $t1, 0($t0)
la $t2, L1
lw $t2, 0($t2)
li $t3, 3
sub $t1, $t2, $t3
sw $t1, 0($t0)
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L0
lw $t1, 0($t0)
la $t2, L0
lw $t2, 0($t2)
li $t3, 3
add $t1, $t2, $t3
sw $t1, 0($t0)
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L0
lw $t1, 0($t0)
li $t2, 3
add $t1, $t1, $t2
sw $t1, 0($t0)
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L12
lw $t1, 0($t0)
la $t2, L0
lw $t2, 0($t2)
li $t3, 3
div $t2, $t3
mflo $t1
sw $t1, 0($t0)
# == print start ==
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, L13
li $v0, 4
syscall
li $a0, 3
li $v0, 1
syscall
la $a0, L14
li $v0, 4
syscall
la $t0, L12
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L0
lw $t1, 0($t0)
la $t2, L12
lw $t2, 0($t2)
li $t3, 3
mul $t1, $t2, $t3
sw $t1, 0($t0)
# == print start ==
la $t0, L12
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, L15
li $v0, 4
syscall
li $a0, 3
li $v0, 1
syscall
la $a0, L16
li $v0, 4
syscall
la $t0, L0
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L18
lw $t1, 0($t0)
la $t2, L17
lw $t2, 0($t2)
li $t3, 3
div $t2, $t3
mfhi $t1
sw $t1, 0($t0)
# == print start ==
la $t0, L17
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, L19
li $v0, 4
syscall
li $a0, 3
li $v0, 1
syscall
la $a0, L20
li $v0, 4
syscall
la $t0, L18
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L17
lw $t1, 0($t0)
li $t2, 1
add $t1, $t1, $t2
sw $t1, 0($t0)
la $t0, L18
lw $t1, 0($t0)
la $t2, L17
lw $t2, 0($t2)
li $t3, 3
div $t2, $t3
mfhi $t1
sw $t1, 0($t0)
# == print start ==
la $t0, L17
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, L21
li $v0, 4
syscall
li $a0, 3
li $v0, 1
syscall
la $a0, L22
li $v0, 4
syscall
la $t0, L18
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L17
lw $t1, 0($t0)
li $t2, 1
add $t1, $t1, $t2
sw $t1, 0($t0)
la $t0, L18
lw $t1, 0($t0)
la $t2, L17
lw $t2, 0($t2)
li $t3, 3
div $t2, $t3
mfhi $t1
sw $t1, 0($t0)
# == print start ==
la $t0, L17
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, L23
li $v0, 4
syscall
li $a0, 3
li $v0, 1
syscall
la $a0, L24
li $v0, 4
syscall
la $t0, L18
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L18
lw $t1, 0($t0)
la $t2, L25
lw $t2, 0($t2)
li $t3, 63
and $t1, $t2, $t3
sw $t1, 0($t0)
# == print start ==
la $a0, L26
li $v0, 4
syscall
la $t0, L17
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, L27
li $v0, 4
syscall
li $a0, 63
li $v0, 1
syscall
la $a0, L28
li $v0, 4
syscall
la $t0, L18
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
la $t0, L17
lw $t1, 0($t0)
li $t2, 1
add $t1, $t1, $t2
sw $t1, 0($t0)
la $t0, L18
lw $t1, 0($t0)
la $t2, L17
lw $t2, 0($t2)
li $t3, 3
div $t2, $t3
mfhi $t1
sw $t1, 0($t0)
# == print start ==
la $t0, L17
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, L29
li $v0, 4
syscall
li $a0, 3
li $v0, 1
syscall
la $a0, L30
li $v0, 4
syscall
la $t0, L18
lw $a0, 0($t0)
li $v0, 1
syscall
la $a0, newline
li $v0, 4
syscall
# == print end ==
# == if-conditional start ==
la $a0, L33
la $a1, L34
jal strcmp
seq $v0, $v0, $zero
beq $v0, $zero, L31
L32:
# == print start ==
la $a0, L35
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L31:
# == if-conditional start ==
la $a0, L38
la $a1, L39
jal strcmp
sne $v0, $v0, $zero
beq $v0, $zero, L36
L37:
# == print start ==
la $a0, L40
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L36:
# == if-conditional start ==
la $a0, L43
la $a1, L44
jal strcmp
slt $v0, $v0, $zero
beq $v0, $zero, L41
L42:
# == print start ==
la $a0, L45
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L41:
# == if-conditional start ==
la $a0, L48
la $a1, L49
jal strcmp
sle $v0, $v0, $zero
beq $v0, $zero, L46
L47:
# == print start ==
la $a0, L50
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L46:
# == if-conditional start ==
la $a0, L53
la $a1, L54
jal strcmp
sgt $v0, $v0, $zero
beq $v0, $zero, L51
L52:
# == print start ==
la $a0, L55
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L51:
# == if-conditional start ==
la $a0, L58
la $a1, L59
jal strcmp
sne $v0, $v0, $zero
beq $v0, $zero, L56
L57:
# == print start ==
la $a0, L60
li $v0, 4
syscall
# == print end ==
# == if-conditional end ==
L56:
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
li $v0, 0
jr $ra

strcmp_lower:
li $v0, -1
jr $ra

strcmp_higher:
li $v0, 1
jr $ra
# ====== END STRCMP ======
