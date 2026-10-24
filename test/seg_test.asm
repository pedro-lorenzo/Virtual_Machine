\\DATA 100
\\EXTRA 0
\\STACK 200
HOLA EQU "Hola"
CHAU EQU "Chau"
otra:   MOV EBX, 1
        STOP
main:   MOV EAX, KS
        ADD EAX, CHAU
        SYS 0xF
        STOP
