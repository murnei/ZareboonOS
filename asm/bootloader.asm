[org 0x7C00]
[bits 16]

start:
    jmp 0x0000:init

init:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7000
    sti
    cld

    mov [boot_drive], dl

    mov word [dap.offset], 0x8000
    mov word [dap.segment], 0x0000
    mov word [dap.sectors], 2
    mov dword [dap.lba_low], 1
    mov dword [dap.lba_high], 0

    mov byte [retries], 3

load_kernel:
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    jnc load_ok

    dec byte [retries]
    jnz load_kernel
    jmp disk_fail

load_ok:
    mov dl, [boot_drive]
    jmp 0x0000:0x8000

disk_fail:
    mov si, disk_error
    call print

halt:
    cli
    hlt
    jmp halt

print:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 7
    int 0x10
    jmp print

.done:
    ret

boot_drive: db 0
retries:    db 0

dap:
    db 0x10
    db 0
.sectors:
    dw 0
.offset:
    dw 0
.segment:
    dw 0
.lba_low:
    dd 0
.lba_high:
    dd 0

disk_error: db "Disk read failed", 0

times 510-($-$$) db 0
dw 0xAA55
