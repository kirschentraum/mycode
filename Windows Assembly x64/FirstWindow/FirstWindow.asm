;program for unicode
UNICODE equ 0

;include files
INCLUDELIB MSVCRT
INCLUDE windows64.inc

.const
szClassName DW L"MyClass",0)
szCaptionMain DW L"My First Window!",0)
szText DW L"Windows x64 Assembly, Simple and Powerful!",0)
szButton DW L"button",0)
szButtonText DW L"&OK",0)
.data?
hInstance DQ ?
.code

;process for the window;
WndProc PROC
	local @stRect:RECT
	local @stPs:PAINTSTRUCT
	local @hdc:QWORD
	local @hWnd:QWORD
	local @uMsg:QWORD
	local @wParam:QWORD
	local @lParam:QWORD
	local @RSP_REMAIN:QWORD; for stack alignment
	;stack alignment for 32 bytes
	mov r10, rsp
	and r10, 1Fh
	mov @RSP_REMAIN, r10
	sub rsp, r10

	sub rsp, 20h
	mov @hWnd, rcx
	mov @uMsg, rdx
	mov @wParam, r8
	mov @lParam, r9
	cmp rdx, WM_CLOSE
	jz message_wm_close
	cmp rdx, wm_paint
	jz message_wm_paint
	cmp rdx, WM_CREATE
	jz message_wm_create

	;default window procedure
	call DefWindowProc
	jmp exit_WndProc

message_wm_create:
	add rsp, 20h
	push 0
	push hInstance 
	push 1
	push @hWnd
	push 22
	push 65
	push 10
	push 10
	sub rsp, 20h
	mov r9, WS_CHILD or WS_VISIBLE
	lea r8, szButtonText
	lea rdx, szButton
	mov rcx, 0
	call CreateWindowEx
	add rsp, 40h	
	xor rax, rax
	jmp exit_WndProc

message_wm_paint:
	lea rdx, @stPs
	mov rcx, @hWnd
	call BeginPaint
	mov @hdc, rax
	lea rdx, @stRect
	mov rcx, @hWnd
	call GetClientRect

	;Draw Text
	sub rsp, 18h
	push DT_SINGLELINE or DT_CENTER or DT_VCENTER
	sub rsp, 20h
	lea r9, @stRect
	mov r8, -1
	lea rdx, szText
	mov rcx, @hdc
	call DrawText
	add rsp, 40h


	lea rdx, @stPs
	mov rcx, @hWnd
	call EndPaint
	xor rax, rax
	jmp exit_WndProc

message_wm_close:
	xor rcx, rcx
	call PostQuitMessage
	xor rax, rax
	jmp exit_WndProc

exit_WndProc:
	add rsp, 20h
	add rsp, @RSP_REMAIN
	ret
WndProc ENDP

;main process
wWinMain PROC
	;local parameters
	local @stWndClass:WNDCLASSEX; window class
	local @stMsg:MSG; message parameter
	local @RSP_REMAIN:QWORD; for stack alignment
	local @nCmdShow:QWORD; nCmdSHow of wWinMain

	;set hInstance and nCmdShow
	mov hInstance, rcx
	mov @nCmdShow, r9
	;stack alignment for 32 bytes
	mov r10, rsp
	and r10, 1Fh
	mov @RSP_REMAIN, r10
	sub rsp, r10

	;Initialize @stWndClass
	mov @stWndClass.cbSize, SIZEOF_WNDCLASSEX
	mov @stWndClass.style, CS_HREDRAW or CS_VREDRAW
	lea rax, WndProc
	mov @stWndClass.lpfnWndProc, rax
	mov @stWndClass.cbClsExtra, 0
	mov @stWndClass.cbWndExtra, 0
	mov @stWndClass.hInstance, rcx
	sub rsp, 20h
	mov rdx, IDI_APPLICATION
	xor rcx, rcx
	call LoadIcon
	mov @stWndClass.hIcon, rax
	mov rdx, IDC_ARROW
	xor rcx, rcx
	call LoadCursor
	mov @stWndClass.hCursor, rax
	mov @stWndClass.hbrBackground, COLOR_WINDOWFRAME
	mov @stWndClass.lpszMenuName, 0
	lea rax, szClassName
	mov @stWndClass.lpszClassName, rax
	lea rcx, @stWndClass
	mov @stWndClass.hIconSm, 0

	;register window class
	call RegisterClassEx
	add rsp, 20h
	push 0
	push hInstance 
	push 0
	push 0
	push 400
	push 600
	push 100
	push 100
	sub rsp, 20h
	mov r9, WS_OVERLAPPEDWINDOW
	lea r8, szCaptionMain
	lea rdx, szClassName
	mov rcx, WS_EX_CLIENTEDGE
	call CreateWindowEx
	add rsp, 40h
	mov rdx, @nCmdShow
	mov rcx, rax
	call ShowWindow
begin_message_loop:
	xor r9, r9
	xor r8, r8
	xor rdx, rdx
	lea rcx, @stMsg
	call GetMessage
	cmp rax, 0
	jz end_message_loop
	lea rcx, @stMsg
	call TranslateMessage
	call DispatchMessage
	jmp begin_message_loop
end_message_loop:
	add rsp, 20h
	add rsp, @RSP_REMAIN
	xor rax, rax
	ret
wWinMain ENDP
END