org 0x7C00
bits 16


; FAT12 header

jmp short start
nop

bdb_oem: db "MSWIN4.1"
bdb_bytes_per_sector: dw 0200h
bdb_sectors_per_cluster: db 1
bdb_reserved_sectors: dw 1
bdb_fat_count: db 2
bdb_dir_entry_count: dw 0E0h
bdb_total_sectors: dw 2880 ; 2880 * 512 = 1.44mb
bdb_media_descriptor_type: db 0F0h ; 3.5 inch floppy disk
bdb_sectors_per_fat: dw 9
bdb_sectors_per_track: dw 18
bdb_heads: dw 2
bdb_hidden_sectors: dd 0
bdb_large_sectors: dd 0

; extended boot sector 
ebr_drive_number: db 0
                  db 0
ebr_signature: db 29h
ebr_volume_id: db 12h, 34h, 56h, 78h ; value doesnt matter
ebr_volume_label: db 'ThoriumOS  ' ; padded to 11 bytes 
ebr_system_id: db 'Fat12   ' ; padded to 8 bytes

start:
   ; setup data segments
   mov ax, 0
   mov ds, ax
   mov es, ax

   ; setup stack
   mov ss, ax
   mov sp, 07C00h

   push es
   push word .after
   retf

.after
   mov [ebr_drive_number], dl ; get drive number from dl

   mov si, msg_loading
   puts

   push es
   mov ah, 08h
   int 013h
   jc floppy_error
   pop es

   and cl, 03Fh ; remove top two bits
   xor ch, ch
   mov [bdb_sectors_per_track], cx

   inc dh
   mov [bdb_heads], dh


   ; compute root dir lba: reserved sectors + num fats * sectors per fat
   mov ax, [bdb_sectors_per_fat]
   mov bl, [bdb_fat_count]
   xor bh, bh
   mul bx ; multiply with ax
   add ax [bdb_reserved_sectors]
   push ax

   ; compute root dir size: (32 * num entries) / bytes per sector
   mov ax, [bdb_dir_entry_count]
   shl ax, 5
   xor dx, dx
   div word [bdb_bytes_per_sector] ; remainder in dx , result in ax

   test dx, dx
   jz .root_dir_read ; if dx == 0 , read root dir
   inc ax ; if dx != 0 add 1 to result

.root_dir_read:
   ; read root dir
   mov cl, al ; root dir size , number of sectors to read
   pop ax ; lba of root dir
   mov dl, [ebr_drive_number]
   mov bx, buffer
   call read_disk

   xor bx, bx
   mov di, buffer

.search_kernel
   mov si, kernel_file
   mov cx, 0Bh ; compare 11 chars
   push di
   repe cmpsb
   pop di
   je .kernel_found

   add di, 32 ; go to next
   inc bx
   cmp bx, [bdb_dir_entry_count]
   jl .search_kernel ; check if still in root dir

   jmp kernel_not_found_error ; if not in root dir anymore throw error

.kernel_found:
   ; nothing for now

; si points to string
puts:
   push si
   push ax

.loop:
   lodsb
   or al, al
   jz .done

   mov ah, 0Eh ; print in tty mode 
   mov bh, 0 ; page number
   int 10h ; call interrupt 16
 
   jmp .loop

.done:
   pop ax
   pop si
   ret


floppy_error:
   mov si, msg_disk_read_error
   call puts
   jmp wait_and_reboot

kernel_not_found_error:
   mov si, msg_kernel_not_found
   call puts
   jmp wait_and_reboot

wait_and_reboot:
   mov ah, 0
   int 16h ; wait for keypress
   jmp 0FFFFh ; jumps to beginiing of bios ; reboots

   hlt

.halt:
   cli
   hlt

; Disk routines

; lba to chs
; in:
;  -ax: lba address
; out;
;  -cx [bits 0-5]: sector number
;  -cx [bits 6-15]: cylinder
;  -dh: head 

lba_to_chs:

   push ax
   push dx

   xor dx, dx ; dx = 0
   div word [bdb_sectors_per_track] ; ax = lba / sectors per track ; dx = lba % sectors per track
   inc dx ; dx++
   mov cx, dx ; sector number

   xor dx, dx
   div word [bdb_heads] ; ax = cylinder ; dx = head 
   mov dh, dl ; dh = head
   mov ch, al ; ch = cylinder (lower 8 bits)
   shl ah, 6
   or cl, ah ; put upper 2 bits of cylinder in cl

   pop ax
   mov dl, al
   pop ax
   ret

; read sectors from disk
; -ax: lba address
; -cl num of sectors to read
; -dl drive number
; -es:bx pointer to read mem
;
read_disk:
   
   push ax
   push bx
   push cx
   push dx
   push di

   push cx ; store cx
   call lba_to_chs ; convert th chs
   pop ax ; al = num of sectors to read

   mov ah, 02h
   mov di, 3

.retry:
   pusha ; save all regs
   stc ; set carry flags
   int 013h ; carry flag cleared => success 
   jnz .done_retry ; exit loop

   ; disk read failed
   popa 
   call disk_reset ;repeat

   dec di 
   test di, di 
   jnz .retry

.fail:
   jmp floppy_error

.done_retry:
   popa

   pop di 
   pop dx
   pop cx
   pop bx
   pop ax

   ret
   

; reset disk controller
; -dl: drive number
disk_reset:
   pusha
   mov ah, 0
   stc 
   int 013h
   jc floppy_error
   popa
   ret


msg_loading: db "Loading...", 0Dh, 0Ah, 0
msg_disk_read_error: db "ERROR: Read from Disk failed!", 0Dh, 0Ah, 0
msg_kernel_not_found: db "ERROR: Kernel not found!", 0Dh, 0Ah, 0
kernel_file: db 'KERNEL  BIN'
kernel_cluster: dw 0

KERNEL_SEGMENT equ 2000h
KERNEL_OFFSET equ 0

times 510-($-$$) db 0
dw 0AA55h

buffer:
