;本程序在控制台打印1-MAX之间的质数表

MAX EQU 1000000;区间最大值
STD_OUTPUT_HANDLE EQU -11;取标准输出设备句柄的常量

;声明使用的api接口
EXTERN GetStdHandle:PROC
EXTERN WriteConsoleA:PROC

;未初始化数据
.data?
string db 21 dup(?);存放转换为字符串的整数
StdHandle dq ?;存放标准输出设备句柄
temp dq ?;临时变量

.data
space db ' ', 0;打印空格的字符串

.code

;过程prime
;功能：判断64位无符号数是否为质数
;参数：rcx
;返回值：如果是质数，rax=1，否则rax=0
;其他受影响寄存器：无
prime PROC
	push r9
	push rdx
	push r8
	push r10
	pushfq;保存现场

	mov r9, 1;r9存放判断结果，默认为质数

	cmp rcx, 2;对0、1、2直接判断
	jb prime_not;0和1不是质数
	jz return;2是质数

	;以下计算rcx>=3时的情况
	mov r8, 2;r8为除数，从2开始逐一试除到r8>=rcx/3；rcx=4仍会执行一次试除
prime_loop:
	mov rax, rcx
	xor rdx, rdx
	div r8
	cmp rdx, 0
	jz prime_not;如果余数为0，跳出循环，返回非质数结果
	inc r8;r8加一
	mov r10, r8
	shl r10, 1
	add r10, r8
	cmp r10, rcx
	ja return;判断r8*3是否大于rcx，若大于rcx则退出循环
	jmp prime_loop

prime_not:
	xor r9, r9;如果不是质数，r9设为0

return:
	mov rax, r9;返回值存入rax

	popfq
	pop r10
	pop r8
	pop rdx
	pop r9;恢复现场
	ret
prime ENDP

;过程unsign2str
;功能：将64位无符号整数按十进制转换为0结尾的字符串，以便输出
;参数：rcx存放整数，rdx存放字符串地址
;返回值：rax存放字符串长度（不包括尾0）
;其他受影响寄存器：无
unsign2str PROC
	push rdi
	push rdx
	push r8
	push r9
	push rcx
	pushfq;保存现场

	xor r8, r8;r8用于存放字符串长度，初始化为0
	mov rdi, rdx;rdi用于存放当前待写入的字符串位置
	mov r9, 10;r9=10
	mov rax, rcx

unsign2str_loop:;本循环从右向左计算十进制每一位数值
	xor rdx, rdx;rdx=0
	div r9;计算rax/10
	add rdx, '0'
	push rdx;将余数转换为字符，并压栈
	inc r8
	cmp rax, 0
	jnz unsign2str_loop;如果rax不等于0，继续循环

	mov rcx, r8;将总位数存入rcx，作为弹栈循环次数
unsign2str_pop:
	pop [rdi]
	inc rdi
	loopnz unsign2str_pop;逐一弹栈，存入字符串[rdi]

	xor r9, r9
	mov [rdi], r9;写入尾0
	mov rax, r8;rax=字符串长度（不含尾0）

	popfq
	pop rcx
	pop r9
	pop r8
	pop rdx
	pop rdi
	ret
unsign2str ENDP

main PROC
	sub rsp, 28h;保留影子空间并对齐堆栈

	mov rcx, STD_OUTPUT_HANDLE
	call GetStdHandle
	mov StdHandle, rax;获取标准输出句柄

	mov r12, 1

main_loop:
	inc r12;从2开始
	cmp r12, MAX
	ja main_return
	mov rcx, r12
	call prime;判断r12是否为质数
	cmp rax, 0
	jz main_loop;如果不是质数，重新开始循环

	lea rdx, string
	call unsign2str;如果是质数，转换为字符串

	lea r9, temp
	mov r8, rax
	mov rcx, StdHandle
	call WriteConsoleA;输出字符串

	lea rdx, space
	lea r9, temp
	mov r8, 1
	mov rcx, StdHandle
	call WriteConsoleA;输出空格

	jmp main_loop;重新开始循环

main_return:

	add rsp, 28h;恢复堆栈
	ret
main ENDP
END