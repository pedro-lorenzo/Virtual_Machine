; parto de la matriz cargada en memoria. Posicion 100 tiene N, de alli en adelante todos los valores de la matriz

; ----- Carga de datos de prueba -----
; Matriz 3x3 simetrica: 1 2 3 / 2 5 -4 / 3 -4 9  -> imprime 1 (la matriz arranca en [108], como usa el codigo)
		MOV [100], 3		; N
		MOV [108], 1		; MAT[0][0]
		MOV [112], 2		; MAT[0][1]
		MOV [116], 3		; MAT[0][2]
		MOV [120], 2		; MAT[1][0]
		MOV [124], 5		; MAT[1][1]
		MOV [128], -4		; MAT[1][2]
		MOV [132], 3		; MAT[2][0]
		MOV [136], -4		; MAT[2][1]
		MOV [140], 9		; MAT[2][2]
; ------------------------------------

		MOV ECX, 0		; i
		MOV EFX, 1		; j
		MOV AC, [100]
		SUB AC,1
OTRO:		CMP ECX,AC
		JZ FIN
		MOV EDX, ECX		; EDX=i
		MUL EDX, [100]		; EDX *= N
		ADD EDX, EFX		; EDX += j
		MUL EDX, 4		; EDX *= TAM
		ADD EDX, 108		; EDX += BASE
		ADD EDX, DS		; Termine de posicionar EDX, que apunta al elemento MAT[i][j]
		MOV EBX, EFX		;EBX=j
		MUL EBX, [100]		;EBX *= N
		ADD EBX, ECX		;EBX+=i
		MUL EBX, 4		;EBX*=4
		ADD EBX, 108		;EBX+=BASE
		ADD EBX, DS		; EBX -> Mat[j][i]
		CMP [EDX],[EBX]
		JP NO_SIMETRICA
		JN NO_SIMETRICA
		ADD EFX, 1
		CMP EFX, [100]
		JZ SIG_FILA
		JMP OTRO
SIG_FILA:	ADD ECX, 1
		MOV EFX, ECX
		ADD EFX, 1
		JMP OTRO
NO_SIMETRICA:	MOV EFX, 0
		JMP IMPRIME
FIN:		MOV EFX, 1		;EFX almacena resultado, si es 1, la matriz es simetrica, si es 0 la matriz no es simetrica
; ----- Impresion del resultado (agregado para testear) -----
IMPRIME:	MOV [DS], EFX
		MOV EDX, DS
		MOV EAX, 1
		LDL ECX, 1
		LDH ECX, 4
		SYS 2
		STOP
		
