BITS 32

SECTION .text

align 4

    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

GLOBAL start

EXTERN kmain

start:
    cli

    call kmain

end:
    hlt
    jmp end