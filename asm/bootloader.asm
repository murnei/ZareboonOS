[org 0x7C00]

start:
   xor ax, ax
   mov es, ax
   mov ds, ax

   mov si, [bot]

   mov bx, 0x8000
   mov ah, 02h
   mov al, 10
   mov ch, 0
   mov cl, 2
   mov dh, 0
   int 0x13
   jc disk_fail

   jmp 0x8000

disk_fail:
   mov si, disk_error
   call print
   hlt

print:
   mov ah, 0x0E
   mov al, [si]
   cmp al, 0
   jz return
   int 0x10
   inc si
   jmp print

return:
   ret

disk_error: db "Disk read failed", 0
bot: db "Bootloader", 0

times 510-($-$$) db 0
dw 0xAA55
