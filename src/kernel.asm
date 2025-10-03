org 0h
bits 16

start:
   mov si, msg_hello_world
   call puts


puts:
   push si
   push ax
   push bx

.loop: 
   lodsb
   or al, al
   jz .done

   mov ah, 0Eh
   mov bh, 0
   int 10h

   jmp .loop

.done
   pop bx
   pop ax
   pop si
   ret


msg_hello_world: db 'HELLO WORLD FROM THE KERNEL', 0Dh, 0Ah, 0
