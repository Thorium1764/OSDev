bits 16

%define fat12 1
%define fat16 2
%define fat32 3
%define ext2  4

; FAT12 header

section .fsjump
   jmp short start
   nop


section .fsheaders

%if (FILESYSTEM == fat12) || (FILESYSTEM == fat16) || (FILESYSTEM == fat32)

   bdb_oem: db 'MSWIN4.1'
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
   bdb_hidden_sectors: db 0
   bdb_large_sectors: db 0

   %if (FILESYSTEM == fat32)
      fat32_sectors_per_fat: dd 0
      fat32_flags: dw 0
      fat32_fat_version_number: dw 0
      fat32_root_dir_cluster: dd 0
      fat32_fsinfo_sector: dw 0
      fat32_backup_boot_sector: dw 0
      fat32_reserved: times 12 db 0
   %endif

   ; extended boot sector 
   ebr_drive_number: db 0
                     db 0
   ebr_signature: db 29h
   ebr_volume_id: db 78h, 56h, 34h, 12h ; value doesnt matter
   ebr_volume_label: db 'ThoriumOS  ' ; padded to 11 bytes 
   ebr_system_id: db 'FAT12   ' ; padded to 8 bytes
%endif

section .entry
   global start

   start:
      ; move partition entry from MBR, so we dont overwrite it
      mov ax PART_ENTRY_SEGMENT
      mov es, ax
      mov di, PART_ENTRY_OFFSET
      mov cx, 16
      rep movsb

      
      ; setup data segments
      mov ax, 0
      mov ds, ax
      mov es, ax

      ; setup stack
      mov ss, ax
      mov sp, 07C00h

      push es
      push word .next
      retf

   .next:
      mov [ebr_drive_number], dl ; get drive number from dl
      
      ; check for extensions
      mov ah, 41h
      mov bx 55AAh
      stc
      int 13h

      jc .no_extensions
      cmp bx, 0AA55h
      jne .no_extensions

   .no_extensions:
      mov byte [have_extensions], 0
   
   .post_ext_check:
      ; load stage 2
      mov si, stage_2_location

      mov ax, STAGE2_SEGMENT
      mov es, ax
      mov bx, STAGE2_OFFSET

   .loop:
      mov eax, [si]
      add si, 4
      mov cl, [si]
      inc si

      cmp eax, 0
      je .done_reading

      call read_disk

      xor ch, ch
      shl cx, 5
      mov di, es
      add di, cx
      mov es, di

      jmp .loop

   .done_reading:
      mov dl, [ebr_drive_number]
      mov si, PART_ENTRY_OFFSET
      mov di PART_ENTRY_SEGMENT

      mov ax, STAGE2_SEGMENT
      mov ds, ax
      mov es, ax

      jmp STAGE2_SEGMENT:STAGE2_OFFSET

      jmp wait_and_reboot ; unreachable

      cli
      hlt

section .text

   ; si points to string
   puts:
      push si
      push ax
      push bx

   .loop:
      lodsb
      or al, al
      jz .done

      mov ah, 0Eh ; print in tty mode 
      mov bh, 0 ; page number
      int 10h ; call interrupt 16
    
      jmp .loop

   .done:
      pop bx
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
      jmp 0FFFFh:0000h ; jumps to beginiing of bios => reboots


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
      
      push eax
      push ax
      push bx
      push cx
      push dx
      push di

      cmp byte [have_extensions], 1
      jne .no_extensions

      ; extensions
      mov [extensions.lba], eax
      mov [extensions.offset], bx
      mov [extensions.count], cl

      mov ah, 42h
      mov si, extensions
      mov di, 3
      jmp .retry

   .no_extensions:

      push cx ; store cx
      call lba_to_chs ; convert th chs
      pop ax ; al = num of sectors to read

      mov ah, 02h
      mov di, 3

   .retry:
      pusha ; save all regs
      stc ; set carry flags
      int 013h ; carry flag cleared => success 
      jnc .done_retry ; exit loop

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
      pop eax

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

section .rodata
   msg_disk_read_error: db "ERROR: Read from Disk failed!", 0Dh, 0Ah, 0
   msg_stage2_not_found: db "ERROR: Second Bootloader stage not found!", 0Dh, 0Ah, 0
   stage2_file: db 'STAGE2  BIN'

section .data
   PART_ENTRY_SEGMENT equ 2000h
   PART_ENTRY_OFFSET equ 0

   STAGE2_SEGMENT equ 0
   STAGE2_OFFSET equ 500h

   have_extensions: db 0
   extensions:
      .size db 10h
            db 0
      .count: dw 0
      .offset: dw 0
      .segment: dw 0
      .lba_addr: dq 0

section .data
   global stage_2_location
   stage_2_location: times 30 db 0

section .bss
   buffer: resb 512
