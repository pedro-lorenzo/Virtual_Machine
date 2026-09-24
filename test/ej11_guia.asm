; supongo que el vector ya esta cargado en memoria, y en la posicion 100 se haya la cantidad de elementos del vector, seguido del vector

; ----- Carga de datos de prueba -----
; Vector: 3, 7, -2, 7, 5, 7  -> maximo 7, 3 apariciones
			MOV [100], 6		; N
			MOV [104], 3		; V[0]
			MOV [108], 7		; V[1]
			MOV [112], -2		; V[2]
			MOV [116], 7		; V[3]
			MOV [120], 5		; V[4]
			MOV [124], 7		; V[5]
; ------------------------------------

		MOV EDX, [100]		; EDX=N
		MOV ECX, 0		; i
		MOV EAX, -999		; EAX, lo utilizo para guardar el maximo
OTRO_1:		CMP ECX, EDX		; i<N
		JNN FIN_1		
		MOV EBX, ECX		; EBX=i
		MUL EBX, 4		; EBX *= 4 (TAM)
		ADD EBX, 104		; EBX += BASE
		ADD EBX, DS
		CMP [EBX], EAX
		JP NUEVO_MAX
		ADD ECX, 1
		JMP OTRO_1
NUEVO_MAX:	MOV EAX, [EBX]
		ADD ECX, 1
		JMP OTRO_1
FIN_1:		MOV ECX, 0
		MOV EFX, 0		;EFX cuenta apariciones del maximo
OTRO_2:		CMP ECX, EDX		; i<N
		JNN FIN_2	
		MOV EBX, ECX		; EBX=i
		MUL EBX, 4		; EBX *= 4 (TAM)
		ADD EBX, 104		; EBX += BASE
		ADD EBX, DS
		ADD ECX, 1		; i++ antes del CMP, para que no pise el CC que evaluan los saltos
		CMP [EBX], EAX
		JP OTRO_2
		JN OTRO_2
		ADD EFX, 1
		JMP OTRO_2
FIN_2:		MOV EDX, DS
		MOV [EDX], EAX
		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
		SYS 2			; imprime maximo
		MOV [EDX], EFX
		SYS 2			;imprime cantidad de apariciones
		STOP
