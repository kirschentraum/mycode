INCLUDELIB MSVCRT
EXTERN ExitProcess:PROC
.code
wWinMain PROC
	xor rax, rax
	mov rbx, 15
	add rax, rbx
	xor rcx, rcx
	call ExitProcess
wWinMain ENDP

END