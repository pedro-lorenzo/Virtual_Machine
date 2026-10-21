SLEN:		;PUSH <<PUNTERO>>
		;CALL SLEN
		;ADD SP, 4
		;devuelve en ECX la cantidad de caracteres
	
		PUSH BP
		MOV BP,SP
		SUB SP, 4		; cont==[BP-4]
		PUSH CC
		MOV ECX, [BP+8]		; ECX=puntero
		MOV [BP-4], 0		; cont= 0 
LOOP_SLEN:	CMP b[ECX], '\0'
		JZ FIN_SLEN
		ADD ECX, 1		; punt++
		ADD [BP-4], 1		; cont ++
		JMP LOOP_SLEN
FIN_SLEN:	MOV ECX, [BP-4]		; ECX=cont
		POP CC
		ADD SP, 4
		MOV SP, BP
		POP BP
		RET
