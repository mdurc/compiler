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
