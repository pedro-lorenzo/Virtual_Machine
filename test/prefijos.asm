\\DATA 32
\\STACK 64
main: MOV EAX, [DS+0]
      MOV EAX, l[DS+0]
      MOV AX, w[DS+2]
      MOV AL, b[DS+4]
      MOV b[DS+5], AH
      MOV w[DS+6], l[DS+10]
      MOV EAX, [EDX]
      MOV EAX, l[EDX]
      SHL EL, 1
      STOP