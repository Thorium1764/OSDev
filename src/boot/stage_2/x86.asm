%macro EnterRealMode 0
   [bits 32]
   jmp word 18h:.pmode16 ; jump to 16 bit protected mode

.pmode16:
   [bits 16]
   mov eax, cr0
   and al, ~1
   mov cr0, eax
   jmp word .rmode

.rmode:
   mov ax, 0
   mov ds, ax
   mov ss, ax
   ; enable interrupts
   sti

%endmacro

%macro EnterProtMode
   cli
   mov eax, cr0
   or al, 1
   mov cr0, eax

   jmp word 08h:.pmode

.pmode:
   [bits 32]

   mov ax, 10h
   mov ds, ax
   mov ss, ax

%endmacro

%macro LinearToSegOffset 4 ; what the 

   mov %3, %1
   shr %3, 4
   mov %2, %4
   mov %3, %1
   and %3, 0xf

%endmacro

global x86_out
x86_out:
   [bits 32]
   mov dx, [esp + 4] ; get the 2 args from the stack and call out on them
   mov al, [esp + 8]
   out dx, al
   ret

global x86_in
x86_in:
   [bits 32]
   mov dx, [esp + 4]
   xor eax, eax
   in al, dx
   ret

global x86_DiskReset
x86_DiskReset:
   [bits 32]

   push ebp
   mov ebp, esp

   EnterRealMode

   mov ah, 0
   mov dl, [bp + 8]
   stc
   int 13h

   mov eax, 1
   sbb eax, 0 ; sub with carry ; 1 on success, 0 on failure

   push eax

   EnterProtMode

   pop eax

   mov esp, ebp
   pop ebp
   ret

global x86_DiskRead
x86_DiskRead:
   push ebp
   mov ebp, esp

   EnterRealMode

   push ebx
   push es

   mov dl, [bp + 8] ; drive
   mov ch, [bp + 12] ; cylinder
   mov cl, [bp + 13] ; other part of cylinder
   shl cl, 6

   mov al, [bp + 16] ; sector
   and al 3Fh ; bits 0-5
   or cl, al

   mov dh, [bp + 20] ; head

   mov al, [bp + 24] ; count

   LinearToSegOffset [bp + 28], es, ebx, bx

   mov ah, 02h
   stc
   int 13h

   mov eax, 1
   sbb eax, 0 ; 1 on success, 0 on fail
   
   pop es
   pop ebx

   push eax

   EnterProtMode

   pop eax

   mov esp, ebp
   pop ebp
   ret

global x86_Disk_GetDriveParams
x86_Disk_GetDriveParams:
   [bits 32]

   push ebp
   mov ebp, esp

   EnterRealMode

   [bits 16]

   push es
   push bx
   push esi
   push di

   mov dl, [bp + 8] ; disk drive
   mov ah, 08h ; literally the same as 8
   mov di 0
   mov es, di
   stc
   int 13h

   mov eax, 1
   sbb eax, 0

   LinearToSegOffset [bp + 12], es, esi, si
   mov [es:si], bl

   mov bl, ch ; cylinders
   mov bh, cl
   shr bh, 6
   inc bx

   LinearToSegOffset [bp + 16], es, esi, si
   mov [es:si], bx

   xor ch, ch ; sectors
   and cl, 3Fh

   LinearToSegOffset [bp + 20], es, esi, si
   mov [es:si], cx

   mov cl, dh ; heads
   inc cx

   LinearToSegOffset [bp + 24], es, esi, si
   mov [es:si], cx

   pop si
   pop esi
   pop bx
   pop es

   push eax

   EnterProtMode
   
   [bits 32]

   pop eax

   mov esp, ebp
   pop ebp
   ret

E820Signature equ 0x534D4150 ; magic numbers

global x86_GetNextE820Block
x86_GetNextE820Block:
   push ebp
   mov ebp, esp

   EnterRealMode

   push ebx
   push ecx
   push edx
   push esi
   push edi
   push ds
   push es

   LinearToSegOffset [bp + 8], es, edi, di ; pointer to struct
   LinearToSegOffset [bp + 12], ds, esi, si ; pointer to continuation id
   mov ebx, [ds:si]
   
   mov eax, 0E820h
   mov edx, E820Signature ; magic number
   mov ecx, 24 ; size of struct
   
   int 15h

   cmp eax, E820Signature
   jne .failure

   .success:
      mov eax, ecx
      mov [ds:si], ebx
      jmp .endif

   .failure:
      mov eax, -1

   .endif:
   
   pop es
   pop ds 
   pop edi
   pop esi
   pop edx
   pop ecx
   pop ebx

   push eax

   EnterProtMode

   pop eax

   mov esp, ebp
   pop ebp
   ret
