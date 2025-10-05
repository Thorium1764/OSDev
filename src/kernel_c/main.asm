bits 16

section _ENTRY class=CODE

extern _centry_
global entry

entry:
   cli
   
   ; setup stack
   mov ax, ds 
   mov ss, ax
   mov sp, 0
   mov bp, sp
   sti


   ;send boot drive as info to c kernel
   xor dh, dh
   push dx
   call _centry_

   cli
   hlt



