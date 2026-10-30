\\STACK 1024
\\include  "list.asm"

lista_orig  equ     "lista original:\n"
lista_nuev  equ     "lista nueva:\n"
head        equ     4
main:       push    bp
            mov     bp, sp
            sub     sp, 4 ; head  (variable local)
            push    ebx

            call    heap_init ;inicializa heap

            sys     0xF

            ; declaro un lista simplemente enlazada (sll)
            ; head->10->20->30->(null)
            mov     [bp-head], null     ; nodo* head = null
            mov     ebx, bp
            sub     ebx, head           ; ebx = &head

            sys     0xF
            
            push    10
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; head = nodo_nuevo(10);
            mov     ebx, [ebx]          ;
            add     ebx, sig            ; ebx = &nodo->sig

            sys     0xF            

            push    20
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; nodo->sig = nodo_nuevo(11);
            mov     ebx, [ebx]          ;
            add     ebx, sig            ; &nodo11->sig   

            sys     0xF
            
            push    30
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; nodo11->sig = nodo_nuevo(12)

            sys     0xF
            
            ; mensaje lista original
            mov     edx, ks
            add     edx, lista_orig
            sys     0x4

            ; imprimir la lista
            push    [bp-head]
            call    list_print          ; list_print (head)
            add     sp, 4       

            sys     0xF

            ; creo nuevo nodo 25
            push    25
            call    nodo_nuevo
            add     sp, 4
            ; eax = puntero a nuevo nodo

            sys     0xF

            ; inserto nuevo nodo ordenado
            mov     ebx, bp
            sub     ebx, head           ; ebx = &head (doble puntero)
            push    eax                 ; puntero a nuevo nodo 
            push    ebx                 ; puntero doble a cabeza de lista
            call    insert_nodo
            add     sp, 8

            sys     0xF

            ; mensaje lista nueva
            mov     edx, ks
            add     edx, lista_nuev
            sys     0x4

            ; imprimir la lista
            push    [bp-head]
            call    list_print          ; list_print (head)
            add     sp, 4  

            sys     0xF

            pop     ebx
            mov     sp, bp 
            pop     bp
            ret