		MOV EAX, 1
		LDL ECX, 2
		LDH ECX, 4
		MOV EDX, DS
		SYS 1
		MOV EBX, [DS]
		MOV ECX, [DS+4]
		CMP ECX, 0	; Si el divisor es 0 no se puede dividir (el loop de restas no terminaria)
		JZ DIV_CERO
		CMP EBX, 0	; Determino signo de cada operando por separado
		JN EBX_NEG
		MOV EDX, 0      ; EDX = bandera signo de EBX (0=positivo)
		JMP SIGUE_EBX
EBX_NEG:	MOV EDX, 1      ; EDX = 1 (negativo)
SIGUE_EBX:	CMP ECX, 0
		JN ECX_NEG
		MOV EFX, 0      ; EFX = bandera signo de ECX (0=positivo)
		JMP SIGUE_ECX
ECX_NEG:	MOV EFX, 1
SIGUE_ECX:	XOR EDX, EFX 	; Combino las dos banderas con XOR, EDX = 1 Si los signos son distintos     
		CMP EBX, 0	; Paso EBX y ECX a valor absoluto 
		JNN SIGUE_ABS_EBX
		NOT EBX
		ADD EBX, 1      ; complemento a 2: invierte el signo
SIGUE_ABS_EBX:	CMP ECX, 0
		JNN SIGUE_ABS_ECX
		NOT ECX
		ADD ECX, 1
SIGUE_ABS_ECX:	MOV EAX, 0 
OTRO:		SUB EBX, ECX
		CMP EBX, 0
		JN RESTO
		ADD EAX, 1
		JMP OTRO
RESTO:		JZ RESTO_CERO
		ADD EBX, ECX
		MOV AC, EBX
		JMP FIN
RESTO_CERO:	MOV AC, 0 
FIN:		CMP EDX, 0	; aplico signo
		JZ ES_POSITIVO    
		NOT EAX		; si no es positivo niego e invierto
		ADD EAX, 1   
ES_POSITIVO:	MOV [DS+8], EAX
		MOV EDX, DS
		ADD EDX, 8
		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
		SYS 2
		STOP
DIV_CERO:	LDH EFX, 0x4552	; 'E' 'R'  (cada MOV a memoria escribe 4 bytes,
		LDL EFX, 0x524F	; 'R' 'O'   asi que se arman los caracteres en un registro)
		MOV [DS+20], EFX
		LDH EFX, 0x5200	; 'R' seguido de ceros
		LDL EFX, 0
		MOV [DS+24], EFX
		MOV EDX, DS
		ADD EDX, 20
		LDL ECX, 5
		LDH ECX, 1
		MOV EAX, 0x02
		SYS 0x02
		STOP
