inicio: mov eax, 0b01 ; seteo para leer en decimal
mov edx, DS ; guardar en el data segment
add edx, 4 ; ...en la posición 1
ldh ecx, 0x04 ; leer valores de 4 bytes
ldl ecx, 0x01 ; leer un solo valor
sys 0x1 ; system call para leer
xor ac, ac ; reseteo el ac (ac = 0)
mov eax, [edx] ; copio 4 bytes de memoria a registro
otro: cmp eax, 0 ; comparo con cero
jz fin ; si es cero terminé
jnn sigue ; si no es negativo salta
add ac, 1 ; si es negativo acumula 1
sigue: shl eax, 1 ; desplazo un bit a la izquierda
jmp otro ; continúa el loop
fin: add edx, 4 ; incremento para usar otra posición
mov [edx], ac ; copia a memoria el ac
mov eax, 0b01 ; seteo para escribir decimal
ldh ecx, 0x04 ; escribir valores de 4 bytes
ldl ecx, 0x01 ; escribir un solo valor
sys 0x2 ; system call para imprimir
stop ; detiene la ejecución
