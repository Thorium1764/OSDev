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
   jmp main


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

main:
   ; setup data segments
   mov ax, 0
   mov ds, ax
   mov es, ax

   ; setup stack
   mov ss, ax
   mov sp, 07C00h

   ; read something from disk
   ; BIOS should put drive number into DL
   mov [ebr_drive_number], dl
   mov ax, 1 ; second disk sector , 0-index
   mov cl, 1 ; 1 sector to read
   mov bx, 07E00h ; 7C00h + 200h = 7E00h
   call read_disk


   ; print message
   mov si, msg_hello_world
   call puts
   
   cli
   hlt

floppy_error:
   mov si, msg_disk_read_error
   call puts
   jmp wait_and_reboot

wait_and_reboot:
   mov ah, 0
   int 16h ; wait for keypress
   jmp 0FFFFh ; jumps to beginiing of bios reboots

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


msg_hello_world: db "HELLO WORLD", 0x0D, 0x0A, 0
msg_disk_read_error: db "Read from Disk failed!", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0AA55h
