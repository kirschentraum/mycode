TITLE ��������

;ϵͳҪ��x64λƽ̨��֧��AVX��CPU

;�����ļ�����input.txt
;�����ʽ��
;�����ݻ� ��Ʒ���� ��Ʒ1����� ��Ʒ1�ļ�ֵ[[��Ʒ2����� ��Ʒ2�ļ�ֵ] ...]
;����ļ�����output.txt

; == == == == == == == == == == == == == == == ���� == == == == == == == == == == == == == == == == == == == == == == == == == =

;���߳���س���
THREAD_NUMBER					EQU 8;												�߳���
THRESHOLD						EQU 10000;											�������߳���ֵ
SIZE_THREAD_PARA				EQU 24;												�̲߳����ṹ��С
INFINITE						EQU 0ffffffffh;										���޵ȴ�

;�ļ����ڴ�ӳ�������س���
FILE_ATTRIBUTE_NORMAL			EQU 80h;											���ļ�������
NULL							EQU 0;												��ָ��
CREATE_ALWAYS					EQU 2;												���ļ���ʽ�������Ƿ����һ�ɴ�
OPEN_EXISTING					EQU 3;												���ļ���ʽ��ֻ���Ѵ��ڵ��ļ�
FILE_SHARE_READ					EQU 1;												�ļ�����ʽ�����������Կ��Զ����ļ�
GENERIC_WRITE					EQU 40000000h;										�ļ����ʷ�ʽ��д
GENERIC_READ					EQU 80000000h;										�ļ����ʷ�ʽ����
INVALID_HANDLE_VALUE			EQU - 1;												��Ч���ļ����
PAGE_READONLY					EQU 2;												�ڴ�ӳ������ԣ�ֻ��
FILE_MAP_READ					EQU 4;												�ļ�ӳ�䵽�ڴ�����ԣ�ֻ��

;read_int������س���
READ_ERROR						EQU 1;												������
DATA_ERROR						EQU 2;												���ݴ���
DATA_TOO_LARGE					EQU 3;												����̫��

;����ַ�������
LEN_BEST_VALUE_HEAD				EQU 34;												������Ž��ܼ�ֵ����
LEN_ITEM_HEAD					EQU 7;												��Ʒ���ǰ׺����
LEN_ITEM_SELECTED				EQU 10;												ѡ�����Ʒ�ĳ���
LEN_ITEM_NOT_SELECTED			EQU 14;												û��ѡ�����Ʒ�ĳ���
LEN_PROCESSING_TIME_TEXT		EQU 19;												����ʱ��ǰ׺����

; == == == == == == == == == == == == == == = �ⲿ�������� == == == == == == == == == == == == == == == == == == == == == == == == == == ==

EXTERN ExitProcess : PROC;												��������
EXTERN CreateFileA : PROC;												���ļ�
EXTERN WriteFile : PROC;												д�ļ�
EXTERN MessageBoxA : PROC;												������Ϣ��
EXTERN GetTickCount : PROC;												��ȡʱ��
EXTERN GetFileSizeEx : PROC;											��ȡ�ļ���С
EXTERN CloseHandle : PROC;												�ر��ļ�
EXTERN CreateFileMappingA : PROC;										�����ļ����ڴ�ӳ��
EXTERN MapViewOfFile : PROC;											���ļ�ӳ�䵽�ڴ�
EXTERN CreateThread : PROC;												�����߳�
EXTERN CreateEventA : PROC;												�����¼�
EXTERN SetEvent : PROC;													�����¼�
EXTERN WaitForMultipleObjects : PROC;									�ȴ�����¼�����
EXTERN WaitForSingleObject : PROC;										�ȴ������¼�����
EXTERN HeapAlloc : PROC;												�Ӷ��з����ڴ�
EXTERN HeapFree : PROC;													�Ӷ����ͷ��ڴ�
EXTERN GetProcessHeap : PROC;											ȡ�öѾ��

; == == == == == == == == == == == == == == = �ṹ���� == == == == == == == == == == == == == == == == == == == == == == == == == == ==
thread_para_struct STRUCT;	�̲߳����ṹ
capacity_min	QWORD ? ;	���̸߳�����ݻ���Сֵ
capacity_max	QWORD ? ;	���̸߳�����ݻ����ֵ
thread_no		QWORD ? ;	���̱߳��
thread_para_struct ENDS

; == == == == == == == == == == == == == == = ���ݲ��� == == == == == == == == == == == == == == == == == == == == == == == == == == ==

.data? ; δ��ʼ������

;�����ݻ�����Ʒ����
total_capacity				QWORD ? ;											�����ݻ�
total_item_number			QWORD ? ;											��Ʒ����

;��������
current_last_select_list	QWORD ? ;											�����Ŀѡ��״̬�б�
item_list					QWORD ? ;											��Ʒ��Ϣ�б�
i_item						QWORD ? ;											��ǰ�������Ʒ��
current_total_value_line	QWORD ? ;											�ܼ�ֵ�б�ĵ�ǰ��
last_total_value_line		QWORD ? ;											�ܼ�ֵ�б����һ��
adjust						QWORD ? ;											ʹ��ջ32λ����Ĳ���

;�ռ�������
last_select_line_length		QWORD ? ;											�����Ŀѡ��״̬�б�ÿ�еĳ���
item_list_memory			QWORD ? ;											��Ʒ��Ϣ�ڴ��ַ
last_select_memory			QWORD ? ;											�����Ŀѡ���б��ڴ��ַ
total_value_memory			QWORD ? ;											�ܼ�ֵ�б��ڴ��ַ
total_value_memory_end		QWORD ? ;											�ܼ�ֵ�б��ڴ��ַĩβ

;���
input_handle				QWORD ? ;											�����ļ����
output_handle				QWORD ? ;											����ļ����
input_map					QWORD ? ;											�����ļ��ڴ�ӳ����
thread_handle				QWORD THREAD_NUMBER dup(? );						���߳�ѭ��������ϵ��¼����
thread_ready_event			QWORD THREAD_NUMBER dup(? );						�߳̾��
all_ready_event				QWORD THREAD_NUMBER dup(? );						�������߳�ѭ��������ϣ���ʼ��һ��ѭ�����¼����
heap_handle					QWORD ? ;											�Ѿ��

;�ļ��й���Ϣ
input_file_size				QWORD ? ;											�����ļ��ֽ���
input_buffer_end			QWORD ? ;											�ڴ�ӳ���β
input_buffer				QWORD ? ;											Ŀǰ�������ڴ�ӳ���ַ
output_buffer				QWORD ? ;											�����������ַ

;�߳��йص�����
width_each_thread			QWORD ? ;											ÿ���̼߳��������С
thread_para 				thread_para_struct THREAD_NUMBER dup(<>);			�̲߳���

.data; �ѳ�ʼ��������

; ȫ���������ı�־
all_finish					DB 0;												���ȫ��������ϣ����߳̽�����Ϊ1

;�ļ���
input_filename				DB 'input.txt', 0;									�����ļ���
output_filename				DB 'output.txt', 0;									����ļ���

;������Ϣ
error_caption				DB 'Error', 0;										������Ϣ�Ի������
file_open_error_message		DB 'File error.', 0;								�ļ��򿪴��������Ϣ
read_error_message			DB 'Read error.', 0;								��ȡ���������Ϣ
write_error_message			DB 'Write error.', 0;								д���������Ϣ
data_error_message			DB 'Data error.', 0;								���ݴ��������Ϣ
data_too_large_message		DB 'Data too large.', 0;							���ݹ��������Ϣ
no_capacity_message			DB 'No capacity.', 0;								�ݻ�Ϊ0�����Ϣ
no_item_message				DB 'No items.', 0;									��Ʒ��Ϊ0�����Ϣ
no_enough_memory_message	DB 'No enough memory.', 0;							�ڴ治����Ϣ
not_support					DB 'Your CPU does not support AVX instructions.', 0;��֧��AVX��Ϣ

;����ı�
best_value_head				DB '---Best Solution---', 0dh, 0ah, 'Total Value: ';����ַ�������һ���ַ���������һ����ʾ
item_head					DB 0dh, 0ah, 'Item ';								ÿ����Ʒ�����뿪ͷ
item_selected				DB ' selected.';									û��ѡ�����Ʒ
item_not_selected			DB ' not selected.';								ѡ�����Ʒ
processing_time_text		DB 0dh, 0ah, 'Processing Time: ';					��������ʱ�����Ϣ
processing_time				DB 22 dup(0);										��������ʱ��
start_caption				DB 'Start', 0;										���㿪ʼ��Ϣ������
start_text					DB 'Calculation start.', 0;							���㿪ʼ��Ϣ������
complete_caption			DB 'Complete', 0;									���������Ϣ������
complete_text				DB 'Calculation complete.', 0;						���������Ϣ������

; == == == == == == == == == == == == == == = ���벿�� == == == == == == == == == == == == == == == == == == == == == == == == == == ==

.code

;�����б�
;main				�����̣���ڵ�
;read_int			���ַ�����ȡ64λ�����Ĺ���
;error_message		��ʾ������Ϣ�Ի���
;hex2str			��64λ�޷�������ת��Ϊ�ַ���
;backpack_in_thread	�̹߳���

; == == == == == == == == == == == == = �����̣���ڵ� == == == == == == == == == == == == == ==

main PROC

	;���CPU�Ƿ�֧��AVXָ�
	mov eax, 1
	cpuid
	test ecx, 10000000h
	jnz avx_check_end
		lea rcx, not_support
		call error_message
		jmp end_main
avx_check_end:
	
	; ��ʾ���㿪ʼ��Ϣ��
	xor r9, r9
	lea r8, start_caption
	lea rdx, start_text
	xor rcx, rcx
	sub rsp, 28h
	call MessageBoxA

	;��ó���ʼִ�е�ʱ��
	call GetTickCount
	add rsp, 28h

	push rax; �������ʼִ�е�ʱ��
	mov adjust, 0
	test rsp, 16
	jz align_check_end
		sub rsp, 16
		mov adjust, 16
align_check_end:
	;�����ڿ�ʼ����ջ�Ѿ�32λ����

	;��input.txt�ļ�
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

	;�����ļ��Ƿ�ɹ�
	cmp rax, INVALID_HANDLE_VALUE
	jnz infile_open_check_end
		lea rcx, file_open_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
infile_open_check_end:

	;���������ļ����
	mov input_handle, rax

	;��ȡ�ļ���С���Ա㰴�ջ�������С����
	sub rsp, 20h
	lea rdx, input_file_size
	mov rcx, rax
	call GetFileSizeEx
	add rsp, 20h
	;����ȡ�ļ���С�Ƿ����
	cmp rax, 0; ������ļ���С����
	jnz file_size_check_end
	cmp input_file_size, 0; ����ļ���СΪ0
	jnz file_size_check_end
		lea rcx, read_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
file_size_check_end:

	;�����ļ����ڴ�ӳ��
	push NULL
	push input_file_size
	sub rsp, 20h
	mov r9d, [OFFSET input_file_size+4]
	mov r8d, PAGE_READONLY
	mov rdx, NULL
	mov rcx, input_handle
	call CreateFileMappingA
	add rsp, 30h
	;��鴴���ڴ�ӳ���Ƿ�ɹ�
	cmp rax, INVALID_HANDLE_VALUE
	jnz create_map_check_end
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
create_map_check_end:

	;����ӳ����
	mov input_map, rax
	sub rsp, 30h
	mov qword ptr [rsp+20h], 0
	xor r9, r9
	xor r8, r8
	mov rdx, FILE_MAP_READ
	mov rcx, rax
	call MapViewOfFile
	add rsp, 30h
	;�����ڴ�ӳ��������յ�
	mov input_buffer, rax
	add rax, input_file_size
	mov input_buffer_end, rax

	;��ȡ�������ݻ�
	lea rcx, total_capacity
	call read_int
	;����read_int����ֵ������
	;���ļ�����
	cmp rax, READ_ERROR
	jnz total_capacity_check_end
		lea rcx, read_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;���ݴ���
	cmp rax, DATA_ERROR
	jnz total_capacity_check_end
		lea rcx, data_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;���ݹ���
	cmp rax, DATA_TOO_LARGE
	jnz total_capacity_check_end
		lea rcx, data_too_large_message
		call error_message
		jmp end_main
	;�����ݻ�Ϊ0
	cmp total_capacity, 0
	jnz total_capacity_check_end
		lea rcx, no_capacity_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
total_capacity_check_end:		

	;��ȡ��Ʒ����
	lea rcx, total_item_number
	call read_int
	;����read_int����ֵ������
	;���ļ�����
	cmp rax, READ_ERROR
	jnz total_item_number_check_end
		lea rcx, read_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;���ݴ���
	cmp rax, DATA_ERROR
	jnz total_item_number_check_end
		lea rcx, data_error_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;���ݹ���
	cmp rax, DATA_TOO_LARGE
	jnz total_item_number_check_end
		lea rcx, data_too_large_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
	;�����ݻ�Ϊ0
	cmp total_item_number, 0
	jnz total_item_number_check_end
		lea rcx, no_item_message
		call error_message
		add rsp, adjust
		pop rax
		jmp end_main
total_item_number_check_end:

	;ȡ�ý��̶Ѿ��
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
	; r13��r14��r15����

	;������Ʒѡ��״̬�б�Ŀռ�
	;���������Ʒѡ��״̬�б��ÿ���ֽ���������last_select_line_length
	;last_select_line_length������32�ı������Ա����λ����
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
	mov r13, rax;��r13ָ�������Ʒѡ��״̬�б�
	cmp r13, 0
	jnz solution_memory_check_end1
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end1:

	;������Ʒ��Ϣ�б�Ŀռ�
	mov rax, rdi
	rol rax, 4
	mov rbx, rax
	test rbx, 15
	jz item_number_large_check_end;���16*total_item_number����2^64-1������ʾ����̫��
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
	mov r15, rax;��r15ָ����Ʒ��Ϣ�б�
	cmp r15, 0
	jnz solution_memory_check_end2
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end2:
	
	;�����ܼ�ֵ�б�Ŀռ�
	mov rax, rsi
	inc rax
	jnc solution_data_large_check3_end
		lea rcx, data_too_large_message
		call error_message
		jmp mem_free
solution_data_large_check3_end:
	rol rax, 4; rax*16�������Ž��б���ֽ���
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
	mov rbx, rax;��rbx�����ܼ�ֵ�б���
	call HeapAlloc
	add rsp, 30h
	mov total_value_memory, rax
	mov r14, rax;��r14ָ���ܼ�ֵ�б�
	add rax, rbx
	mov total_value_memory_end, rax;�����ܼ�ֵ�б��ַ��β
	cmp r14, 0
	jnz solution_memory_check_end3
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end3:

	;�������������
	;Ԥ����Ҫ������ֽ������41 * total_item_number + 100
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
	;��rax����Ϊ16�ı���
	shr rax, 4
	shl rax, 4

	sub rsp, 30h
	xor rdx, rdx
	mov rcx, heap_handle
	mov r8, rax
	call HeapAlloc
	add rsp, 30h
	mov output_buffer, rax;��output_bufferָ�����������
	cmp rax, 0
	jnz solution_memory_check_end5
		lea rcx, no_enough_memory_message
		call error_message
		jmp mem_free
solution_memory_check_end5:

	;��ȡÿ����Ʒ����Ϣ
	;rdiΪѭ��������
	;rcxָʾ��ǰ�������ݵ�ַ�����δ洢����Ʒ1���������Ʒ1�ļ�ֵ����Ʒ2���������Ʒ2�ļ�ֵ����
	add rdi, rdi;�����2*total_item_number������
	mov rcx, r15
item_read_loop:
	cmp rdi, 0; �ж��Ƿ���Ҫ�˳�ѭ��
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
	jmp item_read_loop; ����ѭ��������һ������
close_input:
	;�ر������ļ�
	sub rsp, 20h
	mov rcx, input_handle
	call CloseHandle
	
	;�ر������ļ����ڴ�ӳ��
	mov rcx, input_map
	call CloseHandle
	add rsp, 20h

	;��ʼ�������Ž�

	cmp total_capacity, THRESHOLD
	jc calculation_single; ��������ݻ�������ֵ���õ��̼߳���
	
	;�����ö��̼߳���
	
	;����ÿ���̸߳������ķ�Χ
	mov rax, total_capacity
	xor rdx, rdx
	mov rbx, THREAD_NUMBER
	div rbx
	shr rax, 8
	shl rax, 8;ȷ��ÿ���̸߳���ķ�Χ��256�ı��������������߳�дͬһ���ڴ��ֽ�
	mov	width_each_thread, rax
	;�����̺߳�������
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
	;�����¼����߳�
	xor rsi, rsi
	lea rdi, thread_para; rdiָ��ÿ���̲߳�����ַ
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
		jz calculation_single; ��������¼�ʧ�ܣ����뵥�̼߳���
		mov [rbp], rax

		xor r9, r9
		xor r8, r8
		xor rdx, rdx
		xor rcx, rcx
		call CreateEventA
		cmp rax, 0
		jz calculation_single; ��������¼�ʧ�ܣ����뵥�̼߳���
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
		jz calculation_single; ��������߳�ʧ�ܣ����뵥�̼߳���
		mov [r12], rax
		add rdi, SIZE_THREAD_PARA	
		inc rsi
		add rbp, 8
		add rbx, 8
		add r12, 8
	cmp rsi, THREAD_NUMBER
	jnz create_loop_begin
	
	;���й��б��ַ������ȫ�ֱ����������̷߳���
	mov item_list, r15

	mov r10, r13
	mov r12, r14; r12ָ���ܼ�ֵ�б�ǰ��
	mov r11, r14
	mov rax, total_capacity
	inc rax
	shl rax, 3
	add r11, rax; r11ָ���ܼ�ֵ�б�����һ��

	mov rdi, 1; rdiΪѭ������������ʾ��ǰ������ǵ�rdi����Ʒ
		;������ѭ�����ݣ�֪ͨ�����߳̿�ʼִ��
multi_loop:
		mov current_last_select_list, r10
		mov current_total_value_line, r12
		mov last_total_value_line, r11
		mov i_item, rdi;����ǰ�������Ϣ���浽ȫ�ֱ���
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
		
		;�ȴ�ִ�����
		mov r9, INFINITE
		mov r8, 1
		lea rdx, thread_ready_event
		mov rcx, THREAD_NUMBER
		call WaitForMultipleObjects
		add rsp, 20h
		
		;����ѭ��֮ǰ������
		mov r11, r12
		pop r12; ����r11��r12
		pop r10
		add r10, last_select_line_length; r10����һ��
		inc rdi
	cmp total_item_number, rdi
	jnc multi_loop
	
	mov qword ptr all_finish, 1
	
	;֪ͨ�����߳��˳������رո����
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
	
	;���̼߳���ģ�����

	;���̼߳���ģ�鿪ʼ
calculation_single: 
	
	; r12ָ��ǰ�������Ŀ�����ݻ���Ӧ���ܼ�ֵλ��
	; rbpָ��ǰһ����Ŀ�����ݻ���Ӧ���ܼ�ֵλ��
	; r11ָ��ǰ����������Ʒѡ��״̬λ��
	; rbx��ʾ��ǰ��λ����128λΪ��λ��
	; r10��ʾ��ǰ��λ����64λΪ��λ��
	; r10��r11��r12��rbx��ʼ��
	
	mov r12, r14
	mov r11, r13
	xor r10, r10
	xor rbx, rbx
	xor rcx, rcx
	;��һ��ѭ���������1����Ʒ�����
	mov rdi, 0; rdi��ʾ��ǰ�ݻ�
	
item1_loop_begin:
	;�ж��Ƿ�ѡ���һ����Ʒ
		cmp rdi, [r15]
		jnc set_true

		;�����ѡ��
		;�ܼ�ֵΪ0�������Ʒѡ��״̬��0
			mov qword ptr [r12], 0
			jmp capacity_compare_end

		set_true:
		;���ѡ��;
		;�ܼ�ֵΪ��1����Ʒ�ļ�ֵ�������Ʒѡ��״̬��1
			mov rax, [r15+8]
			mov qword ptr [r12], rax
			bts rcx, r10
		capacity_compare_end:

		;ѭ������ǰ�����ã�rdi����1��r10��r11��r12��Ӧ����
		inc rdi
		add r12, 8
		inc r10
		inc rbx
		cmp r10, 64
		jnz carry_check_end
			test rbx, 64
			jz carry_check_mid1
				pinsrq xmm0, rcx, 0;rbx=64��192
				xor rcx, rcx
				xor r10, r10
				jmp carry_check_end
			carry_check_mid1:;rbx=128��256
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
	jnc item1_loop_begin; �ж��Ƿ���Ҫ�˳�ѭ��
item1_loop_end:

;��ʣ�ಿ�ִ����ڴ棬����r11��λ��ָ����һ�У�����r10����
	cmp rbx, 0
	jz item1_remain_check_end;����0���账��
		cmp rbx, 64
		jnc item1_remain_check_mid1
			mov [r11], rcx; С��64����Ҫ��rcxд���ڴ�
			add r11, 32
			jmp item1_remain_check_end
		item1_remain_check_mid1:
			cmp rbx, 128
			jnc item1_remain_check_mid2
				pinsrq xmm0, rcx, 1; rbx=64-127����Ҫ��rcx��д��xmm0�ĸ�λ����һ��д���ڴ�
				movdqa [r11], xmm0
				add r11, 32
				jmp item1_remain_check_end
			item1_remain_check_mid2:
				cmp rbx, 192
				jnc item_remain_check_mid3
					pinsrq xmm0, rcx, 0;rbx=128-191����Ҫ��rcx��д��xmm0�ĵ�λ����һ��д��ymm0�����д���ڴ�
					vinserti128 ymm1, ymm1, xmm0, 1
					vmovdqa ymmword ptr [r11], ymm1
					add r11, 32
					jmp item1_remain_check_end
				item_remain_check_mid3:
					pinsrq xmm0, rcx, 1;rbx = 192 - 255����Ҫ��rcx��д��xmm0�ĸ�λ����һ��д��ymm0�����д���ڴ�
					vinserti128 ymm1, ymm1, xmm0, 1
					vmovdqa ymmword ptr [r11], ymm1
					add r11, 32
item1_remain_check_end:


;�ڶ���ѭ������������Ʒ�����
; ����rsi�ĳ�ʼֵ��rsi�����ѭ������������ʾ��ǰ�����rsi+1����Ʒ
	mov rsi, 1
	mov rbp, r14; ��һ��ѭ��������r12���Զ�ָ��ڶ��е���ʼλ�ã�ֻҪ��rbpָ���һ����ʼλ�ü���
loop_item_begin:;���ѭ����ʼ����Ʒ������
	; �жϵ�ǰ��Ʒ���Ƿ��Ѿ���������Ʒ������Ҫ�˳�ѭ��
	cmp rsi, total_item_number
	jz loop_item_end
		xor r10, r10; r10��rcx��rbx����
		xor rcx, rcx
		xor rbx, rbx
		
		;r12��rbp���ڲ�ѭ���л��������ƶ����ʱ���r12��rbp��λ�ã��Ա���һ�����ѭ��ʱ����
		push r12
		push rbp

		;��ȡ��ǰ��Ʒ��Ϣ
		;��ѭ���У�r9��ʾ��ǰ��Ʒ�ļ�ֵ��r8��ʾ��ǰ��Ʒ�����
		mov r8, rsi
		shl r8, 4
		add r8, r15
		mov r8, [r8]
		mov r9, rsi
		shl r9, 4
		add r9, r15
		add r9, 8
		mov r9, [r9]

		;����rdi�ĳ�ʼֵ��rdi���ڲ�ѭ������������ʾ��ǰ�ݻ�
		mov rdi, 0

		loop_capacity_begin:; �ڲ�ѭ����ʼ
			cmp total_capacity, rdi
			jc loop_capacity_end; �жϵ�ǰ�ݻ��Ƿ��Ѿ����������ݻ����Ƿ���Ҫ�˳�ѭ��

			;���¼���ǰrsi + 1����Ʒ��rdi�ݻ��µ����Ž�
			;����rdi����ֵ���Ƿ�Ӧѡ��ǰ��Ʒ���������αȽϣ�����������֧����֧1����֧2����֧2-1

			;�Ƚϵ�ǰ��Ʒ���r8�͵�ǰ�ݻ�rdi
			cmp rdi, r8
			jnc current_capacity_not_less

				;��֧1�������ǰ�ݻ�С�ڵ�rsi + 1����Ʒ����������rsi + 1����Ʒ��ѡ��

				mov rax, [rbp]
				mov [r12], rax
				jmp if_branch_end

			current_capacity_not_less:
				;��֧2�������ǰ�ݻ���С�ڵ�rsi + 1����Ʒ�����
				;�Ƚ�r9����ǰrsi����Ʒ��rdi - r8�ݻ��µ����Ž��ܼ�ֵ��rax������ǰrsi����Ʒ��rdi�ݻ��µ����Ž��ܼ�ֵ��rdx��
				mov rdx, r8
				shl rdx, 3
				sub rbp, rdx
				mov rax, [rbp]
				add rax, r9
				add rbp, rdx
				mov rdx, [rbp]

				cmp rdx, rax
				jc current_item_set_true

					;��֧2-1�����r9����ǰrsi����Ʒ��rdi - 8�ݻ��µ����Ž��ܼ�ֵ������ǰrsi����Ʒ��rdi�ݻ��µ����Ž��ܼ�ֵ
					;��ѡ���rsi+1����Ʒ
					mov [r12], rdx
					jmp if_branch_end

		current_item_set_true:
					;��֧2-2�����r9����ǰrsi����Ʒ��rdi - 8�ݻ��µ����Ž��ܼ�ֵ����ǰrsi����Ʒ��rdi�ݻ��µ����Ž��ܼ�ֵ
					;��ѡ���rsi + 1����Ʒ
					mov [r12], rax
					bts rcx, r10;�����Ʒѡ��״̬��1
		if_branch_end:;�������������֧����	

			;�ڲ�ѭ������ǰ������
			;�ݻ�rdi����1, rbx��r10��r11��r12��rbp��Ӧ���ӣ�ָ����λ��
			inc rdi
			add r12, 8
			add rbp, 8
			inc r10
			inc rbx
			cmp r10, 64
			jnz loop_capacity_begin
				test rbx, 64
				jz loop_carry_check_mid1
					pinsrq xmm0, rcx, 0;rbx=64��192
					xor rcx, rcx
					xor r10, r10
					jmp loop_capacity_begin
				loop_carry_check_mid1:;rbx=128��256
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
	loop_capacity_end:; �ڲ�ѭ������

		;���ѭ������ǰ������
		inc rsi
		;��ʣ�ಿ�ִ����ڴ棬����r11��λ��ָ����һ�У�����r10����
		cmp rbx, 0
		jz loop_remain_check_end;����0���账��
			cmp rbx, 64
			jnc loop_remain_check_mid1
				mov [r11], rcx; С��64����Ҫ��rcxд���ڴ�
				add r11, 32
				jmp loop_remain_check_end
			loop_remain_check_mid1:
				cmp rbx, 128
				jnc loop_remain_check_mid2
					pinsrq xmm0, rcx, 1; rbx=64-127����Ҫ��rcx��д��xmm0�ĸ�λ����һ��д���ڴ�
					movdqa [r11], xmm0
					add r11, 32
					jmp loop_remain_check_end
				loop_remain_check_mid2:
					cmp rbx, 192
					jnc loop_remain_check_mid3
						pinsrq xmm0, rcx, 0;rbx=128-191����Ҫ��rcx��д��xmm0�ĵ�λ����һ��д��ymm0�����д���ڴ�
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
						jmp loop_remain_check_end
					loop_remain_check_mid3:
						pinsrq xmm0, rcx, 1;rbx = 192 - 255����Ҫ��rcx��д��xmm0�ĸ�λ����һ��д��ymm0�����д���ڴ�
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
	loop_remain_check_end:

		;�ָ�rbp��r12ָ�����ף�����������
		pop r12
		pop rbp
	jmp loop_item_begin
loop_item_end:; ���ѭ������
	
	;���̼߳���ģ�����
	
output:	
	mov r11, output_buffer;��r11ָ���������������ʼλ��
	mov rdi, r11;��rdiָ������������ĵ�ǰλ��

	;׼�������
	mov rcx, LEN_BEST_VALUE_HEAD
	lea rsi, best_value_head
	cld
	rep movsb;�����Ž��ܼ�ֵǰ׺д�뻺����
	
	;��������Ŀ����ż�ж����Ž��ܼ�ֵ�洢����һ��
	;�����ż����������Ϻ�r12=r14�����Ž���[total_value_memory_end-8]�������������������Ϻ�r12!=r14�����Ž���[r12-8]
	cmp r12, r14
	cmovnz rax, r12
	cmovz rax, total_value_memory_end
	sub rax, 8; ��raxָ�����Ž��ܼ�ֵ���ڵ�λ��

	;�����Ž���ܼ�ֵ��ֵת��Ϊ�ַ�������д�뻺����
	mov rdx, [rax]
	mov rcx, rdi
	call hex2str
	add rdi, rax

	;��������Ʒѡ��״̬�б���������Ž�ĸ���Ʒѡ��״̬
	mov rax, last_select_line_length
	mov rbx, total_item_number
	dec rbx
	mul rbx
	mov rdx, rax
	shl rdx, 3
	add rdx, total_capacity;rdx��ʼ������ʾ����ȡ�������Ʒѡ��״̬�б��λ��
	mov rbx, total_item_number; rbxΪѭ��������
loop_search_begin:
	;ѭ������ÿ����Ŀ��ѡ��״̬������r14ָ���λ�ã����λ��ԭ�����ڴ�����Ž��ܼ�ֵ�б������Ѿ�������Ҫ��
	dec rbx;ѭ����������1
		mov rax, rdx
		mov rcx, rdx
		shr rax, 6
		shl rax, 3; rdx����64���̳���8
		and rcx, 63; rdx����64������
		bt qword ptr [r13+rax], rcx
		setc byte ptr [r14 + rbx]
		jnc select_check_end
		;�����Ʒ�Ѿ�ѡ�񣬲�����һ����Ʒ״̬�����ȵ����ݻ�
		mov rcx, rbx
		shl rcx, 4
		add rcx, r15
		mov rax, [rcx]
		sub rdx, rax
	select_check_end:
		mov rax, last_select_line_length;rdxָ���λ������һ��
		shl rax, 3
		sub rdx, rax
	cmp rbx, 0
	jnz loop_search_begin
	;ѭ������

	;�������Ʒѡ��״̬����r15��Ϊѭ��������
	xor r15, r15
	lea rbp, item_selected
	lea rbx, item_not_selected
	mov r13, LEN_ITEM_SELECTED
	mov r12, LEN_ITEM_NOT_SELECTED
loop_item_select_begin: ;�����Ʒѡ��״̬��ѭ����ʼ
	inc r15; ѭ����������1
		mov rcx, LEN_ITEM_HEAD
		lea rsi, item_head
		cld
		rep movsb

		;����Ʒ���r15תΪ�ַ���
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
		rep movsb;����Ʒѡ��״��д�뻺����

	cmp r15, total_item_number
	jnz loop_item_select_begin; ���r15��ֵ���������total_item_number�����˳�ѭ��

	mov r13, r11;����������ʼλ�ô���r13����Ϊr11���ױ�Ĵ���

	;��output.txt�ļ�
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
	;�����ļ��Ƿ�ɹ�
	cmp rax, INVALID_HANDLE_VALUE
	jnz outfile_open_check_end
		lea rcx, file_open_error_message
		call error_message
		jmp mem_free
	outfile_open_check_end :

	;��������ļ����
	mov output_handle, rax

	sub rdi, r13;������Ҫд����ֽ���
	mov rax, rdi
	xor rdx, rdx
	mov rbx, 0ffffffffh;
	div rbx;
	mov r15, rax;�̴���r15
	mov r14, rdx;��������r14
loop_large_size_begin:; �����д���ֽ���������WriteFile�����������4GB - 1�������д��
	cmp r15, 0
	jz loop_large_size_write_end
		push 0; ʵ��д���ַ���Ŀ��ŵ�ַ��ָ����Ϊ��������
		mov rbp, rsp
		push NULL
		mov r9, rbp
		mov r8, 0ffffffffh
		mov rdx, r13
		mov rcx, output_handle
		sub rsp, 20h
		call WriteFile
		add rsp, 28h
		pop rcx; ��ʵ��д����ֽ���������rcx
		;���д����
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

	push 0; ʵ��д���ַ���Ŀ��ŵ�ַ��ָ����Ϊ��������
	mov rbp, rsp
	push NULL
	mov r9, rbp
	mov r8, r14
	mov rdx, r13
	mov rcx, output_handle
	sub rsp, 20h
	call WriteFile
	add rsp, 28h
	pop rcx; ��ʵ��д����ֽ���������rcx
	;���д����
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
	;�ͷ��ڴ�
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

	;��ȡ����������ϵ�ʱ��
	sub rsp, 20h
	call GetTickCount
	add rsp, 20h
	mov rdx, rax
	;�Ӷ�ջȡ�س���ʼ���е�ʱ��
	add rsp, adjust
	pop rax
	
	;������������ʱ�䣬�Ժ���Ϊ��λ
	sub rdx, rax
	lea rcx, processing_time
	call hex2str
		
	;����������ʱ��д������ļ�
	lea rsi, processing_time
	mov dword ptr[rsi + rax], 2E736D20h;ĩβд��" ms."

	push 0; ��ջ����
	mov rbp, rsp
	push 0; ʵ��д���ַ���Ŀ��ŵ�ַ��ָ����Ϊ��������
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
	pop rcx; ��ʵ��д����ֽ���������rcx
	;���д����
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

	; ��ʾ���������Ϣ��
	xor r9, r9
	lea r8, complete_caption
	lea rdx, complete_text
	xor rcx, rcx
	sub rsp, 28h
	call MessageBoxA
	add rsp, 28h

close_output:	
	;�ر�����ļ�
	sub rsp, 28h
	mov rcx, output_handle
	call CloseHandle
	add rsp, 28h

	;�����̽���

end_main:
	push 0
	xor rcx, rcx
	call ExitProcess

main ENDP

;========================= ���ڴ�ӳ������޷���64λ��������д��[rcx]����rax����ֵ��ʾ�������� ============================

read_int PROC

;�ļ�ΪASCII��ʽ���κη������ַ���һ����Ϊ�ָ���
;���ȡ�ɹ��򷵻�0
;���ļ���ȡ�����򷵻�READ_ERROR
;������ļ�ĩβ��δ�����κ����֣��򷵻�DATA_ERROR
;�����ֳ���2^64-1���򷵻�DATA_TOO_LARGE

	;�����ֳ�
	push rbx
	push rdx
	push r11
	push r12
	push rbp
	
first_digit_loop:
		;��ȡ��һ�����ֵ�ѭ����ֱ��������Ч�����ַ�����ִ���Ϊֹ
		mov rax, input_buffer_end
		cmp rax, input_buffer
		jz data_err;����ļ��Ѿ����꣬�򷵻����ݴ���
		;��ǰ�ַ�����r12�������ڴ�ָ��λ�ü�1
		xor r12, r12
		mov rbp, input_buffer
		mov r12b, [rbp]
		inc input_buffer
	sub r12b, 30h
	jc first_digit_loop;
	cmp r12b, 0ah
	jnc first_digit_loop;�ж϶������ַ��Ƿ���ASCII���֣�������ǣ���������һ���ַ�
	;��һ�����ֶ�ȡѭ������

	;ѭ������λ���֣�����������֣���r12����10�����¶��������֣����û�ж������򷵻ص�ǰr12������
other_digit_loop:
		mov rax, input_buffer_end
		cmp rax, input_buffer
		jz end_read_int; ��������ļ�β��ȻҪ����������ݣ��������������㣩������������̣����ص�ǰr12����ֵ
		;��ǰ�ַ�����r11
		xor r11, r11
		mov rbp, input_buffer
		mov r11b, [rbp]
		inc input_buffer
	sub r11b, 48
	jc end_read_int;
	cmp r11b, 10
	jnc end_read_int;�ж϶������ַ��Ƿ���ASCII���֣�������ǣ���������һ���ַ�

		;����r12*10+r11����Ϊ�µ�r12��ֵ
		mov rax, r12
		mov rbx, 10
		mul rbx
		cmp rdx, 0
		jnz data_large; �����ֵ����2^64-1���򷵻���ֵ̫��Ĵ���
		add rax, r11
		jc data_err
		mov r12, rax; �����ֵ����2 ^ 64 - 1���򷵻���ֵ̫��Ĵ���
	jmp other_digit_loop
	;��λ���ֶ�ȡѭ������

	;���´�����ִ���
read_err:; �ļ�������
	mov rax, READ_ERROR
	jmp return

data_err:; ���ݴ���
	mov rax, DATA_ERROR
	jmp return

data_large:;��ֵ����
	mov rax, DATA_TOO_LARGE
	jmp return

	;��ֵ������ȡ������
end_read_int:
	mov [rcx], r12
	xor rax, rax

	;�ָ��ֳ�
return:
	pop rbp
	pop r12
	pop r11
	pop rdx
	pop rbx
	ret

read_int ENDP

;========================= ��ʾ������Ϣ�Ի���Ĺ��̣�����[rcx]��������ʾ������Ϣ ============================

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

;========================= ��64λ�޷�������ת��Ϊ�ַ����Ĺ��̣�����������rcx��ת�����ַ�����ŵ�ַ��rdx�Ǵ�ת����ֵ������ֵraxΪת������ַ��� ============================
;
hex2str PROC

	;�����ֳ�
	push rbx
	push rcx
	push rdx
	push rdi

	mov rax, rdx; ��ת����ֵ����rax

	;rdx��rdi���㣬rbx��Ϊ10
	xor rdx, rdx
	xor rdi, rdi
	mov rbx, 10

	;��ֵ��λ����10����ת��ΪASCII�ַ���ջ
div_loop_begin:
		div rbx
		add rdx, 48; ����ת��ΪASCII�ַ�
		push rdx; ������ջ
		xor rdx, rdx
		inc rdi; rdi��������λ��
	cmp rax, 0; �������Ƿ�Ϊ0
	jnz div_loop_begin; ����̲���0����ѭ��������һλ

	;�ַ���������rax�У���Ϊ����ֵ
	mov rax, rdi

	;��ÿһλ��γ�ջ����[rcx]
pop_loop_begin : 
		pop rdx
		mov[rcx], dl
		inc rcx
	dec rdi
	jnz pop_loop_begin

	;�ָ��ֳ�
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	ret;

hex2str ENDP;

;========================= �̹߳��� ============================
backpack_in_thread PROC

mov rsi, rcx;rsi�����̺߳���������ַ

thread_proc_begin:
	mov rdx, [rsi+16];��ȡ�̱߳��
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
	mov r14, [rsi+8]; r15��r14�ֱ������̼߳������ֹ��
	mov rdi, r15; rdiΪѭ������������ʾ��ǰ������ݻ�
	mov rdx, item_list
	mov rax, i_item
	dec rax
	shl rax, 4
	add rdx, rax
	mov r8, [rdx]; r9Ϊ��ǰ��Ʒ�����
	mov r9, [rdx+8]; r8Ϊ��ǰ��Ʒ�ļ�ֵ

	mov rdx, r15
	shl rdx, 3
	mov r12, current_total_value_line
	add r12, rdx; r12ָ��ǰ�еĵ�ǰ�ݻ������Ž��ܼ�ֵ�Ĵ洢λ��
	mov rbp, last_total_value_line
	add rbp, rdx; rbpָ����һ�еĵ�ǰ�ݻ������Ž��ܼ�ֵ�Ĵ洢λ��
	shr rdx, 6
	mov r11, current_last_select_list
	add r11, rdx; r11ָ��ǰ�ݻ��ĵ������Ʒѡ��״̬�Ĵ洢�ֽ�
	; rbx��ʾ��ǰ��λ����128λΪ��λ��
	; r10��ʾ��ǰ��λ����64λΪ��λ��
	; rcx�ݴ������Ʒѡ��״̬��ÿ64λ����xmm0����128λ�����ڴ�
	xor r10b, r10b; 
	xor rcx, rcx
	xor rbx, rbx
	
thread_loop_capacity_begin:
	cmp r14, rdi
	jc thread_loop_capacity_end; �жϼ����Ƿ���ɣ����������˳�

		cmp i_item, 1
		jnz more_than_one

			cmp rdi, r8
			jnc one_set_true
			
			;�����ѡ��
			;�ܼ�ֵΪ0�������Ʒѡ��״̬��0
				mov qword ptr [r12], 0
		jmp thread_if_branch_end

			one_set_true:
			;���ѡ��;
			;�ܼ�ֵΪ��1����Ʒ�ļ�ֵ�������Ʒѡ��״̬��1
				mov rax, r9
				mov qword ptr [r12], rax
				bts rcx, r10
	
		jmp thread_if_branch_end
			
		more_than_one:
		
			;��Ʒ������1�Ĳ��֣���������֧
			;�Ƚϵ�ǰ��Ʒ���r8�͵�ǰ�ݻ�rdi
			cmp rdi, r8
			jnc thread_current_capacity_not_less

				;��֧1�������ǰ�ݻ�С�ڵ�ǰ��Ʒ���������ǰ��Ʒ��ѡ��

				mov rax, [rbp]
				mov [r12], rax
			jmp thread_if_branch_end

			thread_current_capacity_not_less:
				;��֧2�������ǰ�ݻ���С�ڵ�i_item����Ʒ�����
				;�Ƚ�r9����ǰrsi����Ʒ��rdi - r8�ݻ��µ����Ž��ܼ�ֵ��rax������ǰrsi����Ʒ��rdi�ݻ��µ����Ž��ܼ�ֵ��rdx��
				mov rdx, r8
				shl rdx, 3
				sub rbp, rdx
				mov rax, [rbp]
				add rax, r9
				add rbp, rdx
				mov rdx, [rbp]

				cmp rdx, rax
				jc thread_current_item_set_true

					;��֧2-1�����r9����ǰi_item-1����Ʒ��rdi-8�ݻ��µ����Ž��ܼ�ֵ������ǰi_item-1����Ʒ��rdi�ݻ��µ����Ž��ܼ�ֵ
					;��ѡ���i_item����Ʒ
					mov [r12], rdx
				jmp thread_if_branch_end

				thread_current_item_set_true:
					;��֧2-2�����r9����ǰi_item-1����Ʒ��rdi - 8�ݻ��µ����Ž��ܼ�ֵ����ǰi_item-1����Ʒ��rdi�ݻ��µ����Ž��ܼ�ֵ
					;��ѡ���i_item����Ʒ
					mov [r12], rax
					bts rcx, r10;�����Ʒѡ��״̬��1
		thread_if_branch_end:;���С��������֧����				
	
		;����rdi��r11��r10
		
		inc rdi
		add r12, 8
		add rbp, 8
		inc r10
		inc rbx
		cmp r10, 64
		jnz thread_loop_capacity_begin
			test rbx, 64
			jz thread_loop_carry_check_mid1
				pinsrq xmm0, rcx, 0;rbx=64��192
				xor rcx, rcx
				xor r10, r10
				jmp thread_loop_capacity_begin
			thread_loop_carry_check_mid1:;rbx=128��256
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
		jz thread_remain_check_end;����0���账��
			cmp rbx, 64
			jnc thread_remain_check_mid1
				mov [r11], rcx; С��64����Ҫ��rcxд���ڴ�
				add r11, 32
				jmp thread_remain_check_end
			thread_remain_check_mid1:
				cmp rbx, 128
				jnc thread_remain_check_mid2
					pinsrq xmm0, rcx, 1; rbx=64-127����Ҫ��rcx��д��xmm0�ĸ�λ����һ��д���ڴ�
					movdqa [r11], xmm0
					add r11, 32
					jmp thread_remain_check_end
				thread_remain_check_mid2:
					cmp rbx, 192
					jnc thread_remain_check_mid3
						pinsrq xmm0, rcx, 0;rbx=128-191����Ҫ��rcx��д��xmm0�ĵ�λ����һ��д��ymm0�����д���ڴ�
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
						jmp thread_remain_check_end
					thread_remain_check_mid3:
						pinsrq xmm0, rcx, 1;rbx = 192 - 255����Ҫ��rcx��д��xmm0�ĸ�λ����һ��д��ymm0�����д���ڴ�
						vinserti128 ymm1, ymm1, xmm0, 1
						vmovdqa ymmword ptr [r11], ymm1
						add r11, 32
thread_remain_check_end:
	;֪ͨ���̣߳����̵߳ļ����Ѿ����
	sub rsp, 28h
	mov rax, [rsi+16];��ȡ�̱߳��
	shl rax, 3
	lea rcx, thread_ready_event
	add rcx, rax
	mov rcx, [rcx]
	call SetEvent
	add rsp, 28h
	
	jmp thread_proc_begin; ׼��������һ����Ʒ��

return:
	ret

backpack_in_thread ENDP

END