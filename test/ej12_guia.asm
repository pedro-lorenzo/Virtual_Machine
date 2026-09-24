; parto de la matriz cargada en memoria. Posicion 100 tiene N, y la 104 M de alli en adelante todos los valores de la matriz

; ----- Carga de datos de prueba -----
; Matriz 3x4:  5 -1  8  2 / 3  4 -6  9 / 7  0  1 -3  -> minimos por columna: 3 -1 -6 -3
			MOV [100], 3		; N (filas)
			MOV [104], 4		; M (columnas)
			MOV [108], 5		; MAT[0][0]
			MOV [112], -1		; MAT[0][1]
			MOV [116], 8		; MAT[0][2]
			MOV [120], 2		; MAT[0][3]
			MOV [124], 3		; MAT[1][0]
			MOV [128], 4		; MAT[1][1]
			MOV [132], -6		; MAT[1][2]
			MOV [136], 9		; MAT[1][3]
			MOV [140], 7		; MAT[2][0]
			MOV [144], 0		; MAT[2][1]
			MOV [148], 1		; MAT[2][2]
			MOV [152], -3		; MAT[2][3]
; ------------------------------------

			MOV ECX, 0		; i
			MOV EFX, 0		; j
			MOV EEX, [100]
			MUL EEX, [104] 		; ALMACENO EN EEX el desplazamiento, donde comienzo a guardar el vector
			MUL EEX, 4		;Tengo que multiplicar por 4 ya que me tengo que desplazar en bytes
			MOV EAX, 0
			MOV EBX, EFX		; EBX=j
			MUL EBX, 4
			ADD EBX, 108
			ADD EBX, EEX		; Me muevo N*M posiciones adelante
			ADD EBX, DS
OTRO_1:			ADD EAX, 1
			MOV [EBX], 9999
			ADD EBX, 4
			CMP EAX, [104]
			JZ OTRO_2
			JMP OTRO_1
OTRO_2:			CMP ECX, [100]
			JZ FIN_RECORRIDO
			MOV EDX, ECX
			MUL EDX, [104]
			ADD EDX, EFX
			MUL EDX, 4
			ADD EDX, 108
			ADD EDX, DS		; Termine de posicionar EDX, que apunta al elemento MAT[i][j]
			MOV EBX, EFX		; EBX=j
			MUL EBX, 4
			ADD EBX, 108
			ADD EBX, EEX		; Me muevo N*M posiciones adelante
			ADD EBX, DS
			ADD EFX, 1
			CMP [EDX], [EBX]
			JN NUEVO_MIN
SIGUE:			CMP [104],EFX
			JZ OTRA_FILA
			JMP OTRO_2
OTRA_FILA:		MOV EFX, 0
			ADD ECX, 1
			JMP OTRO_2
NUEVO_MIN:		MOV [EBX], [EDX]
			JMP SIGUE
FIN_RECORRIDO:		MOV EDX, DS		;al llegar aca complete el vector con los minimos de cada columna
; ----- Impresion del vector de minimos (agregado para testear) -----
			ADD EDX, 108
			ADD EDX, EEX		; EDX apunta al vector (DS + 108 + N*M*4)
			MOV EAX, 1
			MOV ECX, [104]		; M elementos
			LDH ECX, 4		; de 4 bytes
			SYS 2
			STOP
	
	
