includelib libcmt

extern printf:proc

.data
out_string1 db "rand number: %u", 0ah, 0
out_string2 db "loop number: %u", 0ah, 0

.code
main PROC;main
	xor r12, r12
rdloop:
	inc r12
	rdrand r13
	jnc rdloop

	xor r12, r12
rdloop2:
	inc r12
	rdrand r13
	jnc rdloop2

	sub rsp, 28h
	lea rcx, out_string1
	mov rdx, r13
	call printf

	lea rcx, out_string2
	mov rdx, r12
	call printf

	add rsp, 28h
	ret
main ENDP
END