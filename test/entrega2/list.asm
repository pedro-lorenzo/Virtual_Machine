\\include  "heap.asm"
; Singly linked list

; Estructura del nodo:
;    +------------+
;  0 |   entero   |  (2 bytes)
;    +------------+
; +2 | *siguiente |  (4 bytes)
;    +------------+
nodo_size   equ     6
val         equ     0
sig         equ     2

;----------------------------------------
; crea un nuevo nodo de la lista 
; parámetros: +8 valor entero
;----------------------------------------
; invocación:
; push  <valor entero>
; call  nodo_nuevo
; add   sp, 4
; << eax puntero al nuevo nodo
;----------------------------------------
nodo_nuevo:     push    bp
                mov     bp, sp

                push    nodo_size
                call    alloc
                add     sp, 4      

                cmp     eax, null
                jz      nodo_nuevo_fin

                mov     w[eax+val], [bp+8]
                mov     [eax+sig], null

nodo_nuevo_fin: mov     sp, bp
                pop     bp
                ret 


;----------------------------------------
; imprime una lista de valores enteros
; parámentros: +8 puntero a primer nodo
;----------------------------------------
; invocación:
; push  <nodo*>
; call  list_print
; add   sp, 4
;----------------------------------------
list_print:     push    bp
                mov     bp, sp
                push    eax
                push    ecx
                push    edx

                ldh     ecx, 2
                ldl     ecx, 1
                mov     eax, 0x1
                mov     edx, [bp+8]
list_print_imp: cmp     edx, null
                jz      list_print_fin
                sys     0x2
                mov     edx, [edx+sig]
                jmp     list_print_imp

list_print_fin: pop     edx
                pop     ecx
                pop     eax
                mov     sp, bp
                pop     bp
                ret


;----------------------------------------
; insertar un nodo en una lista ordenada
; parametro +8: doble puntero a la lista
; parametro +12: puntero al nodo a insertar
;----------------------------------------
; invocación:
; push <nodo* nuevo_nodo>
; push <nodo** head>
; call insert_nodo
; add  sp, 8
;----------------------------------------
insert_nodo:    push    bp
                mov     bp,sp
                push    eax
                push    ebx
                push    edx

                mov     edx, [bp+8]     ; doble puntero a la lista
                mov     eax, [bp+12]    ; puntero al nodo a insertar
                
                cmp     eax, null       ; si nuevo nodo es null
                jz      insert_fin      ; me voy YA!

                mov     ebx, [edx]      ; puntero a la lista

                cmp     ebx, null
                jz      insert_actual

                cmp     w[eax+val], w[ebx+val]    ; si el nodo a insertar es menor que el primero
                jnp     insert_actual   ; insertar al principio

                add     ebx, sig
                push    eax
                push    ebx
                call    insert_nodo
                add     sp, 8
                jmp     insert_fin

insert_actual:  mov     [eax+sig], [edx]   ; insertar al principio
                mov     [edx], eax
                jmp     insert_fin
insert_fin:     pop     edx
                pop     ebx
                pop     eax
                mov     sp,bp
                pop     bp
                ret
