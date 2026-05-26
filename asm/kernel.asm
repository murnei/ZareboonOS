[org 0x8000]
[bits 16]

start:
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov sp, 0x8000

    mov [boot_drive], dl   ; сохраняем номер диска, с которого загрузились

    mov si, msg_boot
    call print16

    mov ah, 0x02        
    mov al, 40         
    mov ch, 0       
    mov dh, 0    
    mov cl, 4        
    mov dl, [boot_drive]
    mov bx, 0x1000
    mov es, bx
    xor bx, bx         
    int 0x13
    jc disk_err

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp CODE_SEG:pm_entry

disk_err:
    mov si, msg_err
    call print16
.halt: jmp .halt

print16:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print16
.done:
    ret

[bits 32]
pm_entry:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp 0x10000        

gdt_start:
    dq 0x0000000000000000        
gdt_code:
    dw 0xFFFF, 0x0000
    db 0x00, 10011010b, 11001111b, 0x00
gdt_data:
    dw 0xFFFF, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start   ; 0x08
DATA_SEG equ gdt_data - gdt_start   ; 0x10

boot_drive: db 0
msg_boot:   db 'Kernel loaded!', 13, 10, 0
msg_err:    db 'Disk read error!', 0
