;program for unicode
UNICODE equ 0

;include files
INCLUDELIB MSVCRT
INCLUDE windows64.inc

.data?
szBuffer dw 256 dup(?)
hWnd DQ ?


.const
szCaption dw L"Send Message", 0)
szStart dw L"Press OK to start SendMassage, param: %Ix!", 0)
szReturn dw L"SendMessage returned!", 0)
szDestClass dw L"MyClass",0)
szText dw L"Text send to other windows", 0)
szNotFound dw L"Receive Message Window not found!", 0)

.code
wWinMain PROC
	local @hDestWnd:dq
	local @stCopyData:COPYDATASTRUCT
	local @RSP_REMAIN:dq; for stack alignment
	;stack alignment for 32 bytes
	mov r10, rsp
	and r10, 1Fh
	mov @RSP_REMAIN, r10
	sub rsp, r10
	mov	@stCopyData.cbData, 54
	lea rax, szText
	mov	@stCopyData.lpData, rax
	sub rsp, 20h
	;find receiver
	xor rdx, rdx
	lea rcx, szDestClass
	call FindWindow

	cmp rax, 0
	jz	not_found

	mov @hDestWnd, rax
	lea r8, szText
	lea rdx, szStart
	lea rcx, szBuffer
	call wsprintf
	
	mov r9, MB_OK
	lea r8, szCaption
	lea rdx, szBuffer
	xor rcx, rcx	
	call MessageBox

	lea r9, @stCopyData
	xor r8, r8
	mov rdx, WM_COPYDATA
	mov rcx, @hDestWnd
	call SendMessage	

	mov r9, MB_OK
	lea r8, szCaption
	lea rdx, szReturn
	xor rcx, rcx	
	call MessageBox

	jmp end_proc
not_found:
	mov r9, MB_OK
	lea r8, szCaption
	lea rdx, szNotFound
	xor rcx, rcx
	call MessageBox
end_proc:
	add rsp, 20h
	add rsp, @RSP_REMAIN
	xor rax, rax
	ret
wWinMain ENDP
END