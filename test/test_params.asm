\\DATA 16
\\STACK 64
NL EQU "\n"
main:  MOV EAX, [SP+4]
       MOV EBX, [SP+8]
sig:   CMP EAX, 0
       JZ fin
       MOV EDX, [EBX]
       SYS 4
       MOV EDX, KS
       ADD EDX, NL
       SYS 4
       ADD EBX, 4
       SUB EAX, 1
       JMP sig
fin:   RET