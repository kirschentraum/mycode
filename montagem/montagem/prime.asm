;�������ڿ���̨��ӡ1-MAX֮���������

MAX EQU 1000000;�������ֵ
STD_OUTPUT_HANDLE EQU -11;ȡ��׼����豸����ĳ���

;����ʹ�õ�api�ӿ�
EXTERN GetStdHandle:PROC
EXTERN WriteConsoleA:PROC

;δ��ʼ������
.data?
string db 21 dup(?);���ת��Ϊ�ַ���������
StdHandle dq ?;��ű�׼����豸���
temp dq ?;��ʱ����

.data
space db ' ', 0;��ӡ�ո���ַ���

.code

;����prime
;���ܣ��ж�64λ�޷������Ƿ�Ϊ����
;������rcx
;����ֵ�������������rax=1������rax=0
;������Ӱ��Ĵ�������
prime PROC
	push r9
	push rdx
	push r8
	push r10
	pushfq;�����ֳ�

	mov r9, 1;r9����жϽ����Ĭ��Ϊ����

	cmp rcx, 2;��0��1��2ֱ���ж�
	jb prime_not;0��1��������
	jz return;2������

	;���¼���rcx>=3ʱ�����
	mov r8, 2;r8Ϊ��������2��ʼ��һ�Գ���r8>=rcx/3��rcx=4�Ի�ִ��һ���Գ�
prime_loop:
	mov rax, rcx
	xor rdx, rdx
	div r8
	cmp rdx, 0
	jz prime_not;�������Ϊ0������ѭ�������ط��������
	inc r8;r8��һ
	mov r10, r8
	shl r10, 1
	add r10, r8
	cmp r10, rcx
	ja return;�ж�r8*3�Ƿ����rcx��������rcx���˳�ѭ��
	jmp prime_loop

prime_not:
	xor r9, r9;�������������r9��Ϊ0

return:
	mov rax, r9;����ֵ����rax

	popfq
	pop r10
	pop r8
	pop rdx
	pop r9;�ָ��ֳ�
	ret
prime ENDP

;����unsign2str
;���ܣ���64λ�޷���������ʮ����ת��Ϊ0��β���ַ������Ա����
;������rcx���������rdx����ַ�����ַ
;����ֵ��rax����ַ������ȣ�������β0��
;������Ӱ��Ĵ�������
unsign2str PROC
	push rdi
	push rdx
	push r8
	push r9
	push rcx
	pushfq;�����ֳ�

	xor r8, r8;r8���ڴ���ַ������ȣ���ʼ��Ϊ0
	mov rdi, rdx;rdi���ڴ�ŵ�ǰ��д����ַ���λ��
	mov r9, 10;r9=10
	mov rax, rcx

unsign2str_loop:;��ѭ�������������ʮ����ÿһλ��ֵ
	xor rdx, rdx;rdx=0
	div r9;����rax/10
	add rdx, '0'
	push rdx;������ת��Ϊ�ַ�����ѹջ
	inc r8
	cmp rax, 0
	jnz unsign2str_loop;���rax������0������ѭ��

	mov rcx, r8;����λ������rcx����Ϊ��ջѭ������
unsign2str_pop:
	pop [rdi]
	inc rdi
	loopnz unsign2str_pop;��һ��ջ�������ַ���[rdi]

	xor r9, r9
	mov [rdi], r9;д��β0
	mov rax, r8;rax=�ַ������ȣ�����β0��

	popfq
	pop rcx
	pop r9
	pop r8
	pop rdx
	pop rdi
	ret
unsign2str ENDP

main PROC
	sub rsp, 28h;����Ӱ�ӿռ䲢�����ջ

	mov rcx, STD_OUTPUT_HANDLE
	call GetStdHandle
	mov StdHandle, rax;��ȡ��׼������

	mov r12, 1

main_loop:
	inc r12;��2��ʼ
	cmp r12, MAX
	ja main_return
	mov rcx, r12
	call prime;�ж�r12�Ƿ�Ϊ����
	cmp rax, 0
	jz main_loop;����������������¿�ʼѭ��

	lea rdx, string
	call unsign2str;�����������ת��Ϊ�ַ���

	lea r9, temp
	mov r8, rax
	mov rcx, StdHandle
	call WriteConsoleA;����ַ���

	lea rdx, space
	lea r9, temp
	mov r8, 1
	mov rcx, StdHandle
	call WriteConsoleA;����ո�

	jmp main_loop;���¿�ʼѭ��

main_return:

	add rsp, 28h;�ָ���ջ
	ret
main ENDP
END