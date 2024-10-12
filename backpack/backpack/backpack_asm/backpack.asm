TITLE 背包问题

;系统要求：x64位平台，支持AVX的CPU

;输入文件名：input.txt
;输入格式：
;背包容积 物品数量 物品1的体积 物品1的价值[[物品2的体积 物品2的价值] ...]
;输出文件名：output.txt

; == == == == == == == == == == == == == == == 常量 == == == == == == == == == == == == == == == == == == == == == == == == == =

;多线程相关常量
THREAD_NUMBER					EQU 8;												线程数
THRESHOLD						EQU 10000;											开启多线程阈值
SIZE_THREAD_PARA				EQU 24;												线程参数结构大小
INFINITE						EQU 0ffffffffh;										无限等待

;文件和内存映射过程相关常量
FILE_ATTRIBUTE_NORMAL			EQU 80h;											打开文件的属性
NULL							EQU 0;												空指针
CREATE_ALWAYS					EQU 2;												打开文件方式，无论是否存在一律打开
OPEN_EXISTING					EQU 3;												打开文件方式，只打开已存在的文件
FILE_SHARE_READ					EQU 1;												文件共享方式，其他进程仍可以读该文件
GENERIC_WRITE					EQU 40000000h;										文件访问方式，写
GENERIC_READ					EQU 80000000h;										文件访问方式，读
INVALID_HANDLE_VALUE			EQU - 1;												无效的文件句柄
PAGE_READONLY					EQU 2;												内存映射的属性，只读
FILE_MAP_READ					EQU 4;												文件映射到内存的属性，只读

;read_int过程相关常量
READ_ERROR						EQU 1;												读错误
DATA_ERROR						EQU 2;												数据错误
DATA_TOO_LARGE					EQU 3;												数据太大

;输出字符串长度
LEN_BEST_VALUE_HEAD				EQU 34;												输出最优解总价值长度
LEN_ITEM_HEAD					EQU 7;												物品编号前缀长度
LEN_ITEM_SELECTED				EQU 10;												选择该物品的长度
LEN_ITEM_NOT_SELECTED			EQU 14;												没有选择该物品的长度
LEN_PROCESSING_TIME_TEXT		EQU 19;												运行时间前缀长度

; == == == == == == == == == == == == == == = 外部过程声明 == == == == == == == == == == == == == == == == == == == == == == == == == == ==

EXTERN ExitProcess : PROC;												结束程序
EXTERN CreateFileA : PROC;												打开文件
EXTERN WriteFile : PROC;												写文件
EXTERN MessageBoxA : PROC;												弹出信息窗
EXTERN GetTickCount : PROC;												获取时间
EXTERN GetFileSizeEx : PROC;											获取文件大小
EXTERN CloseHandle : PROC;												关闭文件
EXTERN CreateFileMappingA : PROC;										创建文件的内存映射
EXTERN MapViewOfFile : PROC;											将文件映射到内存
EXTERN CreateThread : PROC;												创建线程
EXTERN CreateEventA : PROC;												创建事件
EXTERN SetEvent : PROC;													设置事件
EXTERN WaitForMultipleObjects : PROC;									等待多个事件发生
EXTERN WaitForSingleObject : PROC;										等待单个事件发生
EXTERN HeapAlloc : PROC;												从堆中分配内存
EXTERN HeapFree : PROC;													从堆中释放内存
EXTERN GetProcessHeap : PROC;											取得堆句柄

; == == == == == == == == == == == == == == = 结构声明 == == == == == == == == == == == == == == == == == == == == == == == == == == ==
thread_para_struct STRUCT;	线程参数结构
capacity_min	QWORD ? ;	该线程负责的容积最小值
capacity_max	QWORD ? ;	该线程负责的容积最大值
thread_no		QWORD ? ;	该线程编号
thread_para_struct ENDS

; == == == == == == == == == == == == == == = 数据部分 == == == == == == == == == == == == == == == == == == == == == == == == == == ==

.data? ; 未初始化数据

;背包容积和物品数量
total_capacity				QWORD ? ;											背包容积
total_item_number			QWORD ? ;											物品数量

;计算数据
current_last_select_list	QWORD ? ;											最后项目选择状态列表
item_list					QWORD ? ;											物品信息列表
i_item						QWORD ? ;											当前计算的物品数
current_total_value_line	QWORD ? ;											总价值列表的当前行
last_total_value_line		QWORD ? ;											总价值列表的上一行
adjust						QWORD ? ;											使堆栈32位对齐的参数

;空间分配变量
last_select_line_length		QWORD ? ;											最后项目选择状态列表每行的长度
item_list_memory			QWORD ? ;											物品信息内存地址
last_select_memory			QWORD ? ;											最后项目选择列表内存地址
total_value_memory			QWORD ? ;											总价值列表内存地址
total_value_memory_end		QWORD ? ;											总价值列表内存地址末尾

;句柄
input_handle				QWORD ? ;											输入文件句柄
output_handle				QWORD ? ;											输出文件句柄
input_map					QWORD ? ;											输入文件内存映射句柄
thread_handle				QWORD THREAD_NUMBER dup(? );						子线程循环计算完毕的事件句柄
thread_ready_event			QWORD THREAD_NUMBER dup(? );						线程句柄
all_ready_event				QWORD THREAD_NUMBER dup(? );						所有子线程循环计算完毕，开始新一轮循环的事件句柄
heap_handle					QWORD ? ;											堆句柄

;文件有关信息
input_file_size				QWORD ? ;											输入文件字节数
input_buffer_end			QWORD ? ;											内存映射结尾
input_buffer				QWORD ? ;											目前读到的内存映射地址
output_buffer				QWORD ? ;											输出缓冲区地址

;线程有关的数据
width_each_thread			QWORD ? ;											每个线程计算区间大小
thread_para 				thread_para_struct THREAD_NUMBER dup(<>);			线程参数

.data; 已初始化的数据

; 全部运算计算的标志
all_finish					DB 0;												如果全部计算完毕，主线程将其设为1

;文件名
input_filename				DB 'input.txt', 0;									输入文件名
output_filename				DB 'output.txt', 0;									输出文件名

;错误信息
error_caption				DB 'Error', 0;										错误信息对话框标题
file_open_error_message		DB 'File error.', 0;								文件打开错误输出信息
read_error_message			DB 'Read error.', 0;								读取错误输出信息
write_error_message			DB 'Write error.', 0;								写错误输出信息
data_error_message			DB 'Data error.', 0;								数据错误输出信息
data_too_large_message		DB 'Data too large.', 0;							数据过大输出信息
no_capacity_message			DB 'No capacity.', 0;								容积为0输出信息
no_item_message				DB 'No items.', 0;									物品数为0输出信息
no_enough_memory_message	DB 'No enough memory.', 0;							内存不足信息
not_support					DB 'Your CPU does not support AVX instructions.', 0;不支持AVX信息

;输出文本
best_value_head				DB '---Best Solution---', 0dh, 0ah, 'Total Value: ';这个字符串和下一个字符串连接在一起显示
item_head					DB 0dh, 0ah, 'Item ';								每个物品的输入开头
item_selected				DB ' selected.';									没有选择该物品
item_not_selected			DB ' not selected.';								选择该物品
processing_time_text		DB 0dh, 0ah, 'Processing Time: ';					程序运行时间的信息
processing_time				DB 22 dup(0);										程序运行时间
start_caption				DB 'Start', 0;										计算开始信息窗标题
start_text					DB 'Calculation start.', 0;							计算开始信息窗内容
complete_caption			DB 'Complete', 0;									计算完毕信息窗标题
complete_text				DB 'Calculation complete.', 0;						计算完毕信息窗内容

; == == == == == == == == == == == == == == = 代码部分 == == == == == == == == == == == == == == == == == == == == == == == == == == ==

.code

;过程列表：
;main				主过程，入口点
;read_int			从字符串读取64位整数的过程
;error_message		显示错误信息对话框
;hex2str			将64位无符号整数转换为字符串
;backpack_in_thread	线程过程

; == == == == == == == == == == == == = 主过程，入口点 == == == == == == == == == == == == == ==

main PROC

	;检查CPU是否支持AVX指令集
	mov eax, 1
	cpuid
	test ecx, 10000000h
	jnz avx_check_end
		lea rcx, not_support
		call error_message
		jmp end_main
avx_check_end:
	
	; 显示计算开始信息窗
	xor r9, r9
	lea r8, start_caption
	lea rdx, start_text
	xor rcx, rcx
	sub rsp, 28h
	call MessageBoxA

	;获得程序开始执行的时间
	call GetTickCount
	add rsp, 28h

	push rax; 保存程序开始执行的时间
	mov adjust, 0
	test rsp, 16
	jz align_check_end
		sub rsp, 16
		mov adjust, 16
align_check_end:
	;从现在开始，堆栈已经32位对齐

	;打开input.txt文件
	sub rsp, 40h
	mov qword ptr [rsp+30h], NULL
	mov qword ptr [rsp+28h], FILE_ATTRIBUTE_NORMAL
	mov qword ptr [rsp+20h], OPEN_EXISTING
	xor r9d, r9d
	mov r8d, FILE_SHARE_READ
	mov rdx, GENERIC_READ
	lea rcx, input_filename
	call CreateFileA
	add rsp, 40h

	;检查打开文件是否成功
	cmp rax, INVALID_HANDLE_VALUE
	jnz infile_open_check_end
		lea rcx, file_open_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
infile_open_check_end:

	;储存输入文件句柄
	mov input_handle, rax

	;获取文件大小，以便按照缓冲区大小分区
	sub rsp, 20h
	lea rdx, input_file_size
	mov rcx, rax
	call GetFileSizeEx
	add rsp, 20h
	;检查获取文件大小是否错误
	cmp rax, 0; 如果读文件大小错误
	jnz file_size_check_end
	cmp input_file_size, 0; 如果文件大小为0
	jnz file_size_check_end
		lea rcx, read_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
file_size_check_end:

	;创建文件的内存映射
	push NULL
	push input_file_size
	sub rsp, 20h
	mov r9d, [OFFSET input_file_size+4]
	mov r8d, PAGE_READONLY
	mov rdx, NULL
	mov rcx, input_handle
	call CreateFileMappingA
	add rsp, 30h
	;检查创建内存映射是否成功
	cmp rax, INVALID_HANDLE_VALUE
	jnz create_map_check_end
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
create_map_check_end:

	;储存映射句柄
	mov input_map, rax
	sub rsp, 30h
	mov qword ptr [rsp+20h], 0
	xor r9, r9
	xor r8, r8
	mov rdx, FILE_MAP_READ
	mov rcx, rax
	call MapViewOfFile
	add rsp, 30h
	;计算内存映射的起点和终点
	mov input_buffer, rax
	add rax, input_file_size
	mov input_buffer_end, rax

	;读取背包总容积
	lea rcx, total_capacity
	call read_int
	;根据read_int返回值检查错误
	;读文件错误
	cmp rax, READ_ERROR
	jnz total_capacity_check_end
		lea rcx, read_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;数据错误
	cmp rax, DATA_ERROR
	jnz total_capacity_check_end
		lea rcx, data_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;数据过大
	cmp rax, DATA_TOO_LARGE
	jnz total_capacity_check_end
		lea rcx, data_too_large_message
		call error_message
		jmp end_main
	;背包容积为0
	cmp total_capacity, 0
	jnz total_capacity_check_end
		lea rcx, no_capacity_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
total_capacity_check_end:		

	;读取物品数量
	lea rcx, total_item_number
	call read_int
	;根据read_int返回值检查错误
	;读文件错误
	cmp rax, READ_ERROR
	jnz total_item_number_check_end
		lea rcx, read_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;数据错误
	cmp rax, DATA_ERROR
	jnz total_item_number_check_end
		lea rcx, data_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;数据过大
	cmp rax, DATA_TOO_LARGE
	jnz total_item_number_check_end
		lea rcx, data_too_large_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;背包容积为0
	cmp total_item_number, 0
	jnz total_item_number_check_end
		lea rcx, no_item_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
total_item_number_check_end:

	;取得进程堆句柄
	sub rsp, 30h
	call GetProcessHeap
	add rsp, 30h
	mov heap_handle, rax
	cmp rax, 0
	jnz heap_check_end
		lea rcx, no_enough_memory_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
heap_check_end:
	
	xor r13, r13
	xor r14, r14
	xor r15, r15
	; r13、r14、r15清零

	;分配物品选择状态列表的空间
	;计算最后物品选择状态列表的每行字节数，存入last_select_line_length
	;last_select_line_length必须是32的倍数，以便进行位操作
	mov rsi, total_capacity
	mov rdi, total_item_number
	mov rax, rsi
	shr rax, 8
	inc rax
	shl rax, 5
	mov last_select_line_length, rax
	mov rbx, rdi
	mul rbx
	cmp rdx, 0
	jz solution_data_large_check_end
		lea rcx, data_too_large_message
		call error_message
		jmp mem_free
solution_data_large_check_end:
	
	sub rsp, 30h
	xor rdx, rdx
	mov rcx, heap_handle
	mov r8, rax
	call HeapAlloc
	add rsp, 30h
	mov last_select_memory, rax
	mov r13, rax;令r13指向最后物品选择状态列表
	cmp r13, 0
	jnz solution_memory_check_end1
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end1:

	;分配物品信息列表的空间
	mov rax, rdi
	rol rax, 4
	mov rbx, rax
	test rbx, 15
	jz item_number_large_check_end;如果16*total_item_number超过2^64-1，则提示数据太大
		lea rcx, data_too_large_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
item_number_large_check_end:
	
	sub rsp, 30h
	xor rdx, rdx
	mov rcx, heap_handle
	mov r8, rax
	call HeapAlloc
	add rsp, 30h
	mov item_list_memory, rax
	mov r15, rax;令r15指向物品信息列表
	cmp r15, 0
	jnz solution_memory_check_end2
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end2:
	
	;分配总价值列表的空间
	mov rax, rsi
	inc rax
	jnc solution_data_large_check3_end
		lea rcx, data_too_large_message
		call error_message
		jmp mem_free
solution_data_large_check3_end:
	rol rax, 4; rax*16等于最优解列表的字节数
	mov rbx, rax
	test rbx, 15
	jz solution_data_large_check4_end
		lea rcx, data_too_large_message
		call error_message
		jmp mem_free
solution_data_large_check4_end:

	sub rsp, 30h
	xor rdx, rdx
	mov rcx, heap_handle
	mov r8, rax
	mov rbx, rax;用rbx保存总价值列表长度
	call HeapAlloc
	add rsp, 30h
	mov total_value_memory, rax
	mov r14, rax;令r14指向总价值列表
	add rax, rbx
	mov total_value_memory_end, rax;设置总价值列表地址结尾
	cmp r14, 0
	jnz solution_memory_check_end3
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end3:

	;分配输出缓冲区
	;预估需要输出的字节数最多41 * total_item_number + 100
	mov rax, rdi
	xor rdx, rdx
	mov rbx, 41
	mul rbx
	cmp rdx, 0
	jz output_length_check_end1
		lea rcx, data_too_large_message
		call error_message
		jmp mem_free
output_length_check_end1:
	add rax, 115
	jnc output_length_check_end2
		lea rcx, data_too_large_message
		call error_message
		jmp mem_free
output_length_check_end2:
	;将rax调整为16的倍数
	shr rax, 4
	shl rax, 4

	sub rsp, 30h
	xor rdx, rdx
	mov rcx, heap_handle
	mov r8, rax
	call HeapAlloc
	add rsp, 30h
	mov output_buffer, rax;令output_buffer指向输出缓冲区
	cmp rax, 0
	jnz solution_memory_check_end5
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end5:

	;读取每个物品的信息
	;rdi为循环计数器
	;rcx指示当前读入数据地址，依次存储：物品1的体积，物品1的价值，物品2的体积，物品2的价值……
	add rdi, rdi;须读入2*total_item_number的数据
	mov rcx, r15
item_read_loop:
	cmp rdi, 0; 判断是否需要退出循环
	jz close_input
		call read_int
		cmp rax, READ_ERROR
		jnz item_read_check_end
			lea rcx, read_error_message
			call error_message
			jmp mem_free
		cmp rax, DATA_ERROR
		jnz item_read_check_end
			lea rcx, data_error_message
			call error_message
			jmp mem_free
		cmp rax, DATA_TOO_LARGE
		jnz item_read_check_end
			lea rcx, data_too_large_message
			call error_message
			jmp mem_free
item_read_check_end:
		add rcx, 8
		dec rdi
	jmp item_read_loop; 继续循环，读下一个数据
close_input:
	;关闭输入文件
	sub rsp, 20h
	mov rcx, input_handle
	call CloseHandle
	
	;关闭输入文件的内存映射
	mov rcx, input_map
	call CloseHandle
	add rsp, 20h

	;开始计算最优解

	cmp total_capacity, THRESHOLD
	jc calculation_single; 如果背包容积不足阈值，用单线程计算
	
	;否则用多线程计算
	
	;计算每个线程负责计算的范围
	mov rax, total_capacity
	xor rdx, rdx
	mov rbx, THREAD_NUMBER
	div rbx
	shr rax, 8
	shl rax, 8;确保每个线程负责的范围是256的倍数，以免两个线程写同一个内存字节
	mov	width_each_thread, rax
	;设置线程函数参数
	xor rsi, rsi
	xor rax, rax
	lea rbx, thread_para
set_cal_scope:
		mov [rbx], rax
		add rax, width_each_thread
		mov rdx, rax
		dec rdx
		mov [rbx+8], rdx
		mov [rbx+16], rsi
		add rbx, SIZE_THREAD_PARA
		inc rsi
	cmp rsi, THREAD_NUMBER
	jnz set_cal_scope
	
	sub rbx, 16
	mov rdx, total_capacity
	mov [rbx], rdx
	;创建事件和线程
	xor rsi, rsi
	lea rdi, thread_para; rdi指向每个线程参数地址
	lea rbp, all_ready_event
	lea rbx, thread_ready_event
	lea r12, thread_handle
create_loop_begin:
		sub rsp, 20h
		xor r9, r9
		xor r8, r8
		xor rdx, rdx
		xor rcx, rcx
		call CreateEventA
		cmp rax, 0
		jz calculation_single; 如果创建事件失败，进入单线程计算
		mov [rbp], rax

		xor r9, r9
		xor r8, r8
		xor rdx, rdx
		xor rcx, rcx
		call CreateEventA
		cmp rax, 0
		jz calculation_single; 如果创建事件失败，进入单线程计算
		mov [rbx],rax
		add rsp, 20h
		
		sub rsp, 30h
		mov qword ptr [rsp+28h],0
		mov qword ptr [rsp+20h],0
		mov r9, rdi
		lea r8, backpack_in_thread
		mov rdx, 4096
		xor rcx, rcx
		call CreateThread 
		add rsp, 30h
		cmp rax, 0
		jz calculation_single; 如果创建线程失败，进入单线程计算
		mov [r12], rax
		add rdi, SIZE_THREAD_PARA	
		inc rsi
		add rbp, 8
		add rbx, 8
		add r12, 8
	cmp rsi, THREAD_NUMBER
	jnz create_loop_begin
	
	;将有关列表地址保存在全局变量，供子线程访问
	mov item_list, r15

	mov r10, r13
	mov r12, r14; r12指向总价值列表当前行
	mov r11, r14
	mov rax, total_capacity
	inc rax
	shl rax, 3
	add r11, rax; r11指向总价值列表另外一行

	mov rdi, 1; rdi为循环计数器，表示当前计算的是第rdi个物品
		;待输入循环内容，通知各子线程开始执行
multi_loop:
		mov current_last_select_list, r10
		mov current_total_value_line, r12
		mov last_total_value_line, r11
		mov i_item, rdi;将当前计算的信息保存到全局变量
		push r10
		push r11
		xor rsi, rsi
		lea rbp, all_ready_event
		sub rsp, 20h
	start_loop_begin:
			mov rcx, [rbp]
			call SetEvent
		inc rsi
		add rbp, 8
		cmp rsi, THREAD_NUMBER
		jnz start_loop_begin
		
		;等待执行完毕
		mov r9, INFINITE
		mov r8, 1
		lea rdx, thread_ready_event
		mov rcx, THREAD_NUMBER
		call WaitForMultipleObjects
		add rsp, 20h
		
		;继续循环之前的设置
		mov r11, r12
		pop r12; 交换r11和r12
		pop r10
		add r10, last_select_line_length; r10下移一行
		inc rdi
	cmp total_item_number, rdi
	jnc multi_loop
	
	mov qword ptr all_finish, 1
	
	;通知各子线程退出，并关闭各句柄
	xor rsi, rsi
	lea rbp, all_ready_event
	lea rbx, thread_ready_event
	lea rdi, thread_handle
	sub rsp, 20h
close_loop_begin:
		mov rcx, [rbp]
		call SetEvent
		mov rcx, [rbp]
		call CloseHandle
		mov rcx, [rbx]
		call CloseHandle
		mov rcx, [rdi]
		call CloseHandle
		inc rsi
		add rbp, 8
		add rbx, 8
		add rdi, 8
	cmp rsi, THREAD_NUMBER
	jnz close_loop_begin
	add rsp, 20h
	jmp output
	
	;多线程计算模块结束

	;单线程计算模块开始
calculation_single: 
	
	; r12指向当前计算的项目数和容积对应的总价值位置
	; rbp指向前一个项目数和容积对应的总价值位置
	; r11指向当前计算的最后物品选择状态位置
	; rbx表示当前的位数（128位为单位）
	; r10表示当前的位数（64位为单位）
	; r10、r11、r12、rbx初始化
	
	mov r12, r14
	mov r11, r13
	xor r10, r10
	xor rbx, rbx
	xor rcx, rcx
	;第一个循环，计算第1个物品的情况
	mov rdi, 0; rdi表示当前容积
	
item1_loop_begin:
	;判断是否选择第一个物品
		cmp rdi, [r15]
		jnc set_true

		;如果不选择
		;总价值为0，最后物品选择状态置0
			mov qword ptr [r12], 0
			jmp capacity_compare_end

		set_true:
		;如果选择;
		;总价值为第1个物品的价值，最后物品选择状态置1
			mov rax, [r15+8]
			mov qword ptr [r12], rax
			bts rcx, r10
		capacity_compare_end:

		;循环结束前的设置，rdi增加1，r10、r11、r12相应调整
		inc rdi
		add r12, 8
		inc r10
		inc rbx
		cmp r10, 64
		jnz carry_check_end
			test rbx, 64
			jz carry_check_mid1
				pinsrq xmm0, rcx, 0;rbx=64或192
				xor rcx, rcx
				xor r10, r10
				jmp carry_check_end
			carry_check_mid1:;rbx=128或256
				pinsrq xmm0, rcx, 1
				xor rcx, rcx
				xor r10, r10
				test rbx, 128
				jz carry_check_mid2
					vinserti128 ymm1, ymm1, xmm0, 0;rbx=128
					jmp carry_check_end
				carry_check_mid2:;rbx=256
					vinserti128 ymm1, ymm1, xmm0, 1
					vmovdqa ymmword ptr [r11], ymm1
					add r11, 32
					xor rbx, rbx
	carry_check_end:

	cmp total_capacity, rdi
	jnc item1_loop_begin; 判断是否需要退出循环
item1_loop_end:

;将剩余部分存入内存，调整r11的位置指向下一行，并把r10置零
	cmp rbx, 0
	jz item1_remain_check_end;等于0无需处理
		cmp rbx, 64
		jnc item1_remain_check_mid1
			mov [r11], rcx; 小于64，需要把rcx写入内存
			add r11, 32
			jmp item1_remain_check_end
		item1_remain_check_mid1:
			cmp rbx, 128
			jnc item1_remain_check_mid2
				pinsrq xmm0, rcx, 1; rbx=64-127，需要把rcx先写入xmm0的高位，再一起写入内存
				movdqa [r11], xmm0
				add r11, 32
				jmp item1_remain_check_end
			item1_remain_check_mid2:
				cmp rbx, 192
				jnc item_remain_check_mid3
					pinsrq xmm0, rcx, 0;rbx=128-191，需要把rcx先写入xmm0的低位，再一起写入ymm0，最后写入内存
					vinserti128 ymm1, ymm1, xmm0, 1
					vmovdqa ymmword ptr [r11], ymm1
					add r11, 32
					jmp item1_remain_check_end
				item_remain_check_mid3:
					pinsrq xmm0, rcx, 1;rbx = 192 - 255，需要把rcx先写入xmm0的高位，再一起写入ymm0，最后写入内存
					vinserti128 ymm1, ymm1, xmm0, 1
					vmovdqa ymmword ptr [r11], ymm1
					add r11, 32
item1_remain_check_end:


;第二个循环，计算多个物品的情况
; 设置rsi的初始值，rsi是外层循环计数器，表示当前计算第rsi+1个物品
	mov rsi, 1
	mov rbp, r14; 第一个循环结束后，r12已自动指向第二行的起始位置，只要让rbp指向第一行起始位置即可
loop_item_begin:;外层循环开始，物品数增加
	; 判断当前物品数是否已经超过总物品数，需要退出循环
	cmp rsi, total_item_number
	jz loop_item_end
		xor r10, r10; r10，rcx和rbx清零
		xor rcx, rcx
		xor rbx, rbx
		
		;r12和rbp在内层循环中会随计算而移动；故保存r12和rbp的位置，以便下一轮外层循环时交换
		push r12
		push rbp

		;获取当前物品信息
		;在循环中，r9表示当前物品的价值，r8表示当前物品的体积
		mov r8, rsi
		shl r8, 4
		add r8, r15
		mov r8, [r8]
		mov r9, rsi
		shl r9, 4
		add r9, r15
		add r9, 8
		mov r9, [r9]

		;设置rdi的初始值，rdi是内层循环计数器，表示当前容积
		mov rdi, 0

		loop_capacity_begin:; 内层循环开始
			cmp total_capacity, rdi
			jc loop_capacity_end; 判断当前容积是否已经超过了总容积，是否需要退出循环

			;以下计算前rsi + 1个物品在rdi容积下的最优解
			;根据rdi的数值和是否应选择当前物品，进行两次比较，产生三个分支：分支1，分支2，分支2-1

			;比较当前物品体积r8和当前容积rdi
			cmp rdi, r8
			jnc current_capacity_not_less

				;分支1：如果当前容积小于第rsi + 1个物品的体积，则第rsi + 1个物品不选择

				mov rax, [rbp]
				mov [r12], rax
				jmp if_branch_end

			current_capacity_not_less:
				;分支2：如果当前容积不小于第rsi + 1个物品的体积
				;比较r9加上前rsi个物品在rdi - r8容积下的最优解总价值（rax），和前rsi个物品在rdi容积下的最优解总价值（rdx）
				mov rdx, r8
				shl rdx, 3
				sub rbp, rdx
				mov rax, [rbp]
				add rax, r9
				add rbp, rdx
				mov rdx, [rbp]

				cmp rdx, rax
				jc current_item_set_true

					;分支2-1：如果r9加上前rsi个物品在rdi - 8容积下的最优解总价值不超过前rsi个物品在rdi容积下的最优解总价值
					;则不选择第rsi+1个物品
					mov [r12], rdx
					jmp if_branch_end

		current_item_set_true:
					;分支2-2：如果r9加上前rsi个物品在rdi - 8容积下的最优解总价值大于前rsi个物品在rdi容积下的最优解总价值
					;则选择第rsi + 1个物品
					mov [r12], rax
					bts rcx, r10;最后物品选择状态置1
		if_branch_end:;三个“如果”分支结束	

			;内层循环结束前的设置
			;容积rdi增加1, rbx、r10、r11、r12、rbp相应增加，指向新位置
			inc rdi
			add r12, 8
			add rbp, 8
			inc r10
			inc rbx
			cmp r10, 64
			jnz loop_capacity_begin
				test rbx, 64
				jz loop_carry_check_mid1
					pinsrq xmm0, rcx, 0;rbx=64或192
					xor rcx, rcx
					xor r10, r10
					jmp loop_capacity_begin
				loop_carry_check_mid1:;rbx=128或256
					pinsrq xmm0, rcx, 1
					xor rcx, rcx
					xor r10, r10
					test rbx, 128
					jz loop_carry_check_mid2
						vinserti128 ymm1, ymm1, xmm0, 0;rbx=128
						jmp loop_capacity_begin
					loop_carry_check_mid2:;rbx=256
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
						xor rbx, rbx
						jmp loop_capacity_begin
	loop_capacity_end:; 内层循环结束

		;外层循环结束前的设置
		inc rsi
		;将剩余部分存入内存，调整r11的位置指向下一行，并把r10置零
		cmp rbx, 0
		jz loop_remain_check_end;等于0无需处理
			cmp rbx, 64
			jnc loop_remain_check_mid1
				mov [r11], rcx; 小于64，需要把rcx写入内存
				add r11, 32
				jmp loop_remain_check_end
			loop_remain_check_mid1:
				cmp rbx, 128
				jnc loop_remain_check_mid2
					pinsrq xmm0, rcx, 1; rbx=64-127，需要把rcx先写入xmm0的高位，再一起写入内存
					movdqa [r11], xmm0
					add r11, 32
					jmp loop_remain_check_end
				loop_remain_check_mid2:
					cmp rbx, 192
					jnc loop_remain_check_mid3
						pinsrq xmm0, rcx, 0;rbx=128-191，需要把rcx先写入xmm0的低位，再一起写入ymm0，最后写入内存
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
						jmp loop_remain_check_end
					loop_remain_check_mid3:
						pinsrq xmm0, rcx, 1;rbx = 192 - 255，需要把rcx先写入xmm0的高位，再一起写入ymm0，最后写入内存
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
	loop_remain_check_end:

		;恢复rbp和r12指向行首，并交换二者
		pop r12
		pop rbp
	jmp loop_item_begin
loop_item_end:; 外层循环结束
	
	;单线程计算模块结束
	
output:	
	mov r11, output_buffer;令r11指向输出缓冲区的起始位置
	mov rdi, r11;令rdi指向输出缓冲区的当前位置

	;准备输出答案
	mov rcx, LEN_BEST_VALUE_HEAD
	lea rsi, best_value_head
	cld
	rep movsb;将最优解总价值前缀写入缓冲区
	
	;根据总项目数奇偶判断最优解总价值存储在哪一行
	;如果是偶数，计算完毕后r12=r14，最优解在[total_value_memory_end-8]；如果是奇数，计算完毕后r12!=r14，最优解在[r12-8]
	cmp r12, r14
	cmovnz rax, r12
	cmovz rax, total_value_memory_end
	sub rax, 8; 令rax指向最优解总价值所在的位置

	;将最优解的总价值数值转换为字符串，并写入缓冲区
	mov rdx, [rax]
	mov rcx, rdi
	call hex2str
	add rdi, rax

	;从最终物品选择状态列表，计算出最优解的各物品选择状态
	mov rax, last_select_line_length
	mov rbx, total_item_number
	dec rbx
	mul rbx
	mov rdx, rax
	shl rdx, 3
	add rdx, total_capacity;rdx初始化，表示待读取的最后物品选择状态列表的位置
	mov rbx, total_item_number; rbx为循环计数器
loop_search_begin:
	;循环查找每个项目的选择状态，存入r14指向的位置（这个位置原来用于存放最优解总价值列表，现在已经不再需要）
	dec rbx;循环计数器减1
		mov rax, rdx
		mov rcx, rdx
		shr rax, 6
		shl rax, 3; rdx除以64的商乘以8
		and rcx, 63; rdx除以64的余数
		bt qword ptr [r13+rax], rcx
		setc byte ptr [r14 + rbx]
		jnc select_check_end
		;如果物品已经选择，查找下一个物品状态须首先调整容积
		mov rcx, rbx
		shl rcx, 4
		add rcx, r15
		mov rax, [rcx]
		sub rdx, rax
	select_check_end:
		mov rax, last_select_line_length;rdx指向的位置上移一行
		shl rax, 3
		sub rdx, rax
	cmp rbx, 0
	jnz loop_search_begin
	;循环结束

	;输出各物品选择状态，以r15作为循环计数器
	xor r15, r15
	lea rbp, item_selected
	lea rbx, item_not_selected
	mov r13, LEN_ITEM_SELECTED
	mov r12, LEN_ITEM_NOT_SELECTED
loop_item_select_begin: ;输出物品选择状态的循环开始
	inc r15; 循环计数器加1
		mov rcx, LEN_ITEM_HEAD
		lea rsi, item_head
		cld
		rep movsb

		;将物品编号r15转为字符串
		mov rdx, r15
		mov rcx, rdi
		call hex2str
		add rdi, rax

		mov rdx, r14
		add rdx, r15
		dec rdx
		mov rax, [rdx]
		test rax, 1
		cmovnz rsi, rbp
		cmovz rsi, rbx
		cmovnz rcx, r13
		cmovz rcx, r12
		cld
		rep movsb;将物品选择状况写入缓冲区

	cmp r15, total_item_number
	jnz loop_item_select_begin; 检查r15的值，如果等于total_item_number，则退出循环

	mov r13, r11;将缓冲区起始位置存入r13，因为r11是易变寄存器

	;打开output.txt文件
	sub rsp, 40h
	mov qword ptr [rsp+30h], NULL
	mov qword ptr [rsp+28h], FILE_ATTRIBUTE_NORMAL
	mov qword ptr [rsp+20h], CREATE_ALWAYS
	xor r9d, r9d
	mov r8d, FILE_SHARE_READ
	mov rdx, GENERIC_WRITE
	lea rcx, output_filename
	call CreateFileA
	add rsp, 40h
	;检查打开文件是否成功
	cmp rax, INVALID_HANDLE_VALUE
	jnz outfile_open_check_end
		lea rcx, file_open_error_message
		call error_message
		jmp mem_free
	outfile_open_check_end :

	;储存输出文件句柄
	mov output_handle, rax

	sub rdi, r13;计算需要写入的字节数
	mov rax, rdi
	xor rdx, rdx
	mov rbx, 0ffffffffh;
	div rbx;
	mov r15, rax;商存入r15
	mov r14, rdx;余数存入r14
loop_large_size_begin:; 如果待写入字节数超过了WriteFile函数最大上限4GB - 1，须分批写入
	cmp r15, 0
	jz loop_large_size_write_end
		push 0; 实际写入字符数目存放地址，指针作为参数传递
		mov rbp, rsp
		push NULL
		mov r9, rbp
		mov r8, 0ffffffffh
		mov rdx, r13
		mov rcx, output_handle
		sub rsp, 20h
		call WriteFile
		add rsp, 28h
		pop rcx; 将实际写入的字节数弹出至rcx
		;检查写错误
		cmp rax, 0
		jnz write_loop_check_end1
			lea rcx, write_error_message
			call error_message
			jmp close_output
	write_loop_check_end1:
		cmp rcx, rdi
		jz write_loop_check_end2
			lea rcx, write_error_message
			call error_message
			jmp close_output
	write_loop_check_end2:
	dec r15
	jmp loop_large_size_begin
loop_large_size_write_end:

	push 0; 实际写入字符数目存放地址，指针作为参数传递
	mov rbp, rsp
	push NULL
	mov r9, rbp
	mov r8, r14
	mov rdx, r13
	mov rcx, output_handle
	sub rsp, 20h
	call WriteFile
	add rsp, 28h
	pop rcx; 将实际写入的字节数弹出至rcx
	;检查写错误
	cmp rax, 0
	jnz write_check_end1
		lea rcx, write_error_message
		call error_message
		jmp close_output
write_check_end1 :
	cmp rcx, rdi
	jz write_check_end2
		lea rcx, write_error_message
		call error_message
		jmp close_output
write_check_end2:

mem_free:
	;释放内存
	mov r8, item_list_memory
	cmp r8, 0
	jz mem_free2
		sub rsp, 30h
		mov rcx, heap_handle
		xor rdx, rdx
		call HeapFree
		add rsp, 30h
mem_free2:
	mov r8, last_select_memory
	cmp r8, 0
	jz mem_free3
		sub rsp, 30h
		mov rcx, heap_handle
		xor rdx, rdx
		call HeapFree
		add rsp, 30h
mem_free3:
	mov r8, total_value_memory
	cmp r8, 0
	jz mem_free4
		sub rsp, 30h
		mov rcx, heap_handle
		xor rdx, rdx
		call HeapFree
		add rsp, 30h
mem_free4:
	mov r8, output_buffer
	cmp r8, 0
	jz mem_free5
		sub rsp, 30h
		mov rcx, heap_handle
		xor rdx, rdx
		call HeapFree
		add rsp, 30h
mem_free5:

	;获取程序运行完毕的时间
	sub rsp, 20h
	call GetTickCount
	add rsp, 20h
	mov rdx, rax
	;从堆栈取回程序开始运行的时间
	add rsp, adjust
	pop rax
	
	;计算程序的运行时间，以毫秒为单位
	sub rdx, rax
	lea rcx, processing_time
	call hex2str
		
	;将程序运行时间写入输出文件
	lea rsi, processing_time
	mov dword ptr[rsi + rax], 2E736D20h;末尾写入" ms."

	push 0; 堆栈对齐
	mov rbp, rsp
	push 0; 实际写入字符数目存放地址，指针作为参数传递
	mov rdi, LEN_PROCESSING_TIME_TEXT + 4
	add rdi, rax
	push NULL
	mov r9, rbp
	mov r8, rdi
	lea rdx, processing_time_text
	mov rcx, output_handle
	sub rsp, 20h
	call WriteFile
	add rsp, 30h
	pop rcx; 将实际写入的字节数弹出至rcx
	;检查写错误
	cmp rax,0
	jnz write_prossing_time_check_end1
		lea rcx, write_error_message
		call error_message
		jmp close_output

write_prossing_time_check_end1:
	cmp rcx, rdi
	jz write_prossing_time_check_end2
		lea rcx,write_error_message
		call error_message
		jmp close_output
		write_prossing_time_check_end2:

	; 显示计算完毕信息窗
	xor r9, r9
	lea r8, complete_caption
	lea rdx, complete_text
	xor rcx, rcx
	sub rsp, 28h
	call MessageBoxA
	add rsp, 28h

close_output:	
	;关闭输出文件
	sub rsp, 28h
	mov rcx, output_handle
	call CloseHandle
	add rsp, 28h

	;主过程结束

end_main:
	push 0
	xor rcx, rcx
	call ExitProcess

main ENDP

;========================= 从内存映射读入无符号64位整数，并写入[rcx]，用rax返回值表示错误类型 ============================

read_int PROC

;文件为ASCII格式，任何非数字字符，一律视为分隔符
;如读取成功则返回0
;如文件读取错误，则返回READ_ERROR
;如读至文件末尾仍未读到任何数字，则返回DATA_ERROR
;如数字超过2^64-1，则返回DATA_TOO_LARGE

	;保护现场
	push rbx
	push rdx
	push r11
	push r12
	push rbp
	
first_digit_loop:
		;读取第一个数字的循环，直到读到有效数字字符或出现错误为止
		mov rax, input_buffer_end
		cmp rax, input_buffer
		jz data_err;如果文件已经读完，则返回数据错误
		;当前字符载入r12，并将内存指针位置加1
		xor r12, r12
		mov rbp, input_buffer
		mov r12b, [rbp]
		inc input_buffer
	sub r12b, 30h
	jc first_digit_loop;
	cmp r12b, 0ah
	jnc first_digit_loop;判断读到的字符是否是ASCII数字，如果不是，继续读下一个字符
	;第一个数字读取循环结束

	;循环读高位数字，如果读到数字，则将r12乘以10加上新读到的数字，如果没有读到，则返回当前r12的数字
other_digit_loop:
		mov rax, input_buffer_end
		cmp rax, input_buffer
		jz end_read_int; 如果读到文件尾仍然要求继续读数据（即数据数量不足），则结束本过程，返回当前r12的数值
		;当前字符载入r11
		xor r11, r11
		mov rbp, input_buffer
		mov r11b, [rbp]
		inc input_buffer
	sub r11b, 48
	jc end_read_int;
	cmp r11b, 10
	jnc end_read_int;判断读到的字符是否是ASCII数字，如果不是，继续读下一个字符

		;计算r12*10+r11，作为新的r12数值
		mov rax, r12
		mov rbx, 10
		mul rbx
		cmp rdx, 0
		jnz data_large; 如果数值超过2^64-1，则返回数值太大的错误
		add rax, r11
		jc data_err
		mov r12, rax; 如果数值超过2 ^ 64 - 1，则返回数值太大的错误
	jmp other_digit_loop
	;高位数字读取循环结束

	;以下处理各种错误
read_err:; 文件读错误
	mov rax, READ_ERROR
	jmp return

data_err:; 数据错误
	mov rax, DATA_ERROR
	jmp return

data_large:;数值过大
	mov rax, DATA_TOO_LARGE
	jmp return

	;数值正常读取，返回
end_read_int:
	mov [rcx], r12
	xor rax, rax

	;恢复现场
return:
	pop rbp
	pop r12
	pop r11
	pop rdx
	pop rbx
	ret

read_int ENDP

;========================= 显示错误信息对话框的过程，按照[rcx]的内容显示错误信息 ============================

error_message PROC

	sub rsp, 28h
	xor r9d, r9d
	lea r8, error_caption
	mov rdx, rcx
	xor rcx, rcx
	call MessageBoxA
	add rsp, 28h
	ret;

error_message ENDP

;========================= 将64位无符号整数转换为字符串的过程，两个参数：rcx是转换后字符串存放地址，rdx是待转换数值；返回值rax为转换后的字符数 ============================
;
hex2str PROC

	;保护现场
	push rbx
	push rcx
	push rdx
	push rdi

	mov rax, rdx; 待转换数值存入rax

	;rdx和rdi清零，rbx设为10
	xor rdx, rdx
	xor rdi, rdi
	mov rbx, 10

	;数值逐位除以10，并转换为ASCII字符入栈
div_loop_begin:
		div rbx
		add rdx, 48; 余数转换为ASCII字符
		push rdx; 余数入栈
		xor rdx, rdx
		inc rdi; rdi保存结果的位数
	cmp rax, 0; 测试商是否为0
	jnz div_loop_begin; 如果商不是0继续循环，求下一位

	;字符数保存在rax中，作为返回值
	mov rax, rdi

	;将每一位逐次出栈存入[rcx]
pop_loop_begin : 
		pop rdx
		mov[rcx], dl
		inc rcx
	dec rdi
	jnz pop_loop_begin

	;恢复现场
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	ret;

hex2str ENDP;

;========================= 线程过程 ============================
backpack_in_thread PROC

mov rsi, rcx;rsi保存线程函数参数地址

thread_proc_begin:
	mov rdx, [rsi+16];获取线程编号
	shl rdx, 3
	lea rax, all_ready_event
	add rax, rdx
	mov edx, INFINITE
	mov rcx, [rax]
	sub rsp, 28h
	call WaitForSingleObject
	add rsp, 28h

	cmp all_finish, 0
	jnz return
	
	mov r15, [rsi]
	mov r14, [rsi+8]; r15和r14分别是子线程计算的起止点
	mov rdi, r15; rdi为循环计数器，表示当前计算的容积
	mov rdx, item_list
	mov rax, i_item
	dec rax
	shl rax, 4
	add rdx, rax
	mov r8, [rdx]; r9为当前物品的体积
	mov r9, [rdx+8]; r8为当前物品的价值

	mov rdx, r15
	shl rdx, 3
	mov r12, current_total_value_line
	add r12, rdx; r12指向当前行的当前容积的最优解总价值的存储位置
	mov rbp, last_total_value_line
	add rbp, rdx; rbp指向上一行的当前容积的最优解总价值的存储位置
	shr rdx, 6
	mov r11, current_last_select_list
	add r11, rdx; r11指向当前容积的的最后物品选择状态的存储字节
	; rbx表示当前的位数（128位为单位）
	; r10表示当前的位数（64位为单位）
	; rcx暂存最后物品选择状态，每64位存入xmm0，满128位存入内存
	xor r10b, r10b; 
	xor rcx, rcx
	xor rbx, rbx
	
thread_loop_capacity_begin:
	cmp r14, rdi
	jc thread_loop_capacity_end; 判断计算是否完成，如果完成则退出

		cmp i_item, 1
		jnz more_than_one

			cmp rdi, r8
			jnc one_set_true
			
			;如果不选择
			;总价值为0，最后物品选择状态置0
				mov qword ptr [r12], 0
		jmp thread_if_branch_end

			one_set_true:
			;如果选择;
			;总价值为第1个物品的价值，最后物品选择状态置1
				mov rax, r9
				mov qword ptr [r12], rax
				bts rcx, r10
	
		jmp thread_if_branch_end
			
		more_than_one:
		
			;物品数大于1的部分，有三个分支
			;比较当前物品体积r8和当前容积rdi
			cmp rdi, r8
			jnc thread_current_capacity_not_less

				;分支1：如果当前容积小于当前物品的体积，则当前物品不选择

				mov rax, [rbp]
				mov [r12], rax
			jmp thread_if_branch_end

			thread_current_capacity_not_less:
				;分支2：如果当前容积不小于第i_item个物品的体积
				;比较r9加上前rsi个物品在rdi - r8容积下的最优解总价值（rax），和前rsi个物品在rdi容积下的最优解总价值（rdx）
				mov rdx, r8
				shl rdx, 3
				sub rbp, rdx
				mov rax, [rbp]
				add rax, r9
				add rbp, rdx
				mov rdx, [rbp]

				cmp rdx, rax
				jc thread_current_item_set_true

					;分支2-1：如果r9加上前i_item-1个物品在rdi-8容积下的最优解总价值不超过前i_item-1个物品在rdi容积下的最优解总价值
					;则不选择第i_item个物品
					mov [r12], rdx
				jmp thread_if_branch_end

				thread_current_item_set_true:
					;分支2-2：如果r9加上前i_item-1个物品在rdi - 8容积下的最优解总价值大于前i_item-1个物品在rdi容积下的最优解总价值
					;则选择第i_item个物品
					mov [r12], rax
					bts rcx, r10;最后物品选择状态置1
		thread_if_branch_end:;所有“如果”分支结束				
	
		;调整rdi、r11、r10
		
		inc rdi
		add r12, 8
		add rbp, 8
		inc r10
		inc rbx
		cmp r10, 64
		jnz thread_loop_capacity_begin
			test rbx, 64
			jz thread_loop_carry_check_mid1
				pinsrq xmm0, rcx, 0;rbx=64或192
				xor rcx, rcx
				xor r10, r10
				jmp thread_loop_capacity_begin
			thread_loop_carry_check_mid1:;rbx=128或256
				pinsrq xmm0, rcx, 1
				xor rcx, rcx
				xor r10, r10
				test rbx, 128
				jz thread_loop_carry_check_mid2
					vinserti128 ymm1, ymm1, xmm0, 0;rbx=128
					jmp thread_loop_capacity_begin
				thread_loop_carry_check_mid2:;rbx=256
					vinserti128 ymm1, ymm1, xmm0, 1
					vmovdqa ymmword ptr [r11], ymm1
					add r11, 32
					xor rbx, rbx
					jmp thread_loop_capacity_begin
thread_loop_capacity_end:	
		cmp rbx, 0
		jz thread_remain_check_end;等于0无需处理
			cmp rbx, 64
			jnc thread_remain_check_mid1
				mov [r11], rcx; 小于64，需要把rcx写入内存
				add r11, 32
				jmp thread_remain_check_end
			thread_remain_check_mid1:
				cmp rbx, 128
				jnc thread_remain_check_mid2
					pinsrq xmm0, rcx, 1; rbx=64-127，需要把rcx先写入xmm0的高位，再一起写入内存
					movdqa [r11], xmm0
					add r11, 32
					jmp thread_remain_check_end
				thread_remain_check_mid2:
					cmp rbx, 192
					jnc thread_remain_check_mid3
						pinsrq xmm0, rcx, 0;rbx=128-191，需要把rcx先写入xmm0的低位，再一起写入ymm0，最后写入内存
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
						jmp thread_remain_check_end
					thread_remain_check_mid3:
						pinsrq xmm0, rcx, 1;rbx = 192 - 255，需要把rcx先写入xmm0的高位，再一起写入ymm0，最后写入内存
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
thread_remain_check_end:
	;通知主线程，本线程的计算已经完成
	sub rsp, 28h
	mov rax, [rsi+16];获取线程编号
	shl rax, 3
	lea rcx, thread_ready_event
	add rcx, rax
	mov rcx, [rcx]
	call SetEvent
	add rsp, 28h
	
	jmp thread_proc_begin; 准备计算下一个物品数

return:
	ret

backpack_in_thread ENDP

END