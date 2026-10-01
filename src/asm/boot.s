BITS 32

SECTION .text

align 4

    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

GLOBAL start

EXTERN kmain
EXTERN multiboot_magic
EXTERN multiboot_info_addr

start:
    cli

    ; Save Multiboot values into shared globals.
    mov [multiboot_magic], eax
    mov [multiboot_info_addr], ebx

    call kmain

end:
    hlt
    jmp end