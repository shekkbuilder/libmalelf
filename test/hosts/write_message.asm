BITS 32
%include "inc/util.inc.asm"

        prologue
        push dword 0x000a4b34
        push dword 0x49205942
        push dword 0x2044454e
        push word 0x574f

        mov ecx, esp
        mov edx, 13
        mov ebx, 1
        mov eax, 4
        int 0x80

        xor eax, eax
        xor ebx, ebx
        xor ecx, ecx
        xor edx, edx

        epilogue
