#include <Windows.h>
#include <stdio.h>
//�������⣬������x64ƽ̨
//�����ļ�����input.txt
//�����ʽ��
//�����ݻ� ��Ʒ���� ��Ʒ1����� ��Ʒ1�ļ�ֵ [[��Ʒ2����� ��Ʒ2�ļ�ֵ] ...]
//����ļ�����output.txt

const unsigned int thread_number = 8;//�߳���
const unsigned long long threshold = 10000;//�������̵߳ı����ݻ���ֵ

HANDLE thread[thread_number];//�߳̾��
HANDLE thread_ready_event[thread_number];//���߳�ѭ��������ϵ��¼����
HANDLE all_ready_event[thread_number];//�������߳�ѭ��������ϣ���ʼ��һ��ѭ�����¼����

char* input_string, * output_string;;//ָ�������ļ����ݺ�����ļ����ݵ��ַ���
unsigned long long total_capacity, total_item_number;//�����ݻ�����Ʒ����
HANDLE input_handle, output_handle;//�����ļ����������ļ����
unsigned long input_filesize_low, input_filesize_high;//�����ļ���λ�͵�λ��С
unsigned long long input_filesize;//���������ļ��ܴ�С
HANDLE input_map;//�����ļ����ڴ�ӳ����
HANDLE heap_handle;//�Ѿ��

unsigned long long i_item;//��ǰ�������Ʒ��
bool all_finish;//��ʾ���м��������Ѿ����

struct thread_parameter_struct
{
	unsigned long long capacity_min, capacity_max;
	unsigned int thread_no;
};

thread_parameter_struct thread_para[thread_number];//���̲߳���

struct item//ÿ����Ʒ����Ϣ
{
	unsigned long long volume, value;
};

item* item_list;//��Ʒ��Ϣ�б�
unsigned long long* total_value_list;//�ܼ�ֵ�б�
__int8* last_select_list;//���һ����Ʒѡ������б�
bool* final_select_list;//���ս���ȫ����Ʒѡ������б�
unsigned long long last_select_line_length;//���һ����Ʒѡ������б��ÿ�г���

inline bool read_int(unsigned long long* result)//�ӳ���Ϊinput_filesize���ַ���input_string����һ��64λ�޷��������ĺ��������ص�result
												//����������ַ�����β��Ȼ�������κ����֣��򷵻�false�����򷵻�true
{
	static unsigned long long string_pointer{ 0 };//��ǰ��ȡ���ַ�λ��
	unsigned long long value = 0;
	do//����һ�����ֵ�ѭ��
	{
		if (string_pointer >= input_filesize)//����Ѿ�������β��δ�������֣����ش���
			return false;
		value = unsigned long long(input_string[string_pointer]) - 48;
		string_pointer++;
	} while (value > 9);

	if (string_pointer >= input_filesize)
	{
		*result = value;
		return true;
	}
	while (input_string[string_pointer] >= '0' && input_string[string_pointer] <= '9')//���������ֵ�ѭ��
	{
		value = value * 10 + (input_string[string_pointer]) - 48;
		string_pointer++;
		if (string_pointer >= input_filesize)
		{
			*result = value;
			return true;
		}
	}
	*result = value;
	return true;
}

inline bool item_solution_init()//��Ʒ��Ϣ�б��ܼ�ֵ�б�����һ����Ʒѡ������б��ʼ��������ڴ治���򷵻�false
{
	item_list = (item*)HeapAlloc(heap_handle, 0, (total_item_number)*sizeof(item));
	if (item_list == NULL)
		return false;
	final_select_list = (bool*)HeapAlloc(heap_handle, 0, (total_item_number)*sizeof(bool));
	if (final_select_list == NULL)
		return false;
	total_value_list = (unsigned long long *)HeapAlloc(heap_handle, 0, 2 * (total_capacity + 1) * sizeof(unsigned long long));//���Ž��ܼ�ֵֻ������������2�У��Խ�ʡ�ڴ�
	if (total_value_list == NULL)
		return false;
	last_select_line_length = total_capacity / 64 * 8 + 8;
	last_select_list = (char*)HeapAlloc(heap_handle, 0, last_select_line_length * total_item_number);//�������һ����Ʒѡ������б�������ֽ���
	if (last_select_list == NULL)
		return false;
	return true;
}

inline void item_solution_delete()//�ͷ��ѷ�����ڴ�
{
	if (item_list != NULL)
		HeapFree(heap_handle, 0, item_list);
	if (final_select_list != NULL)
		HeapFree(heap_handle, 0, final_select_list);
	if (total_value_list != NULL)
		HeapFree(heap_handle, 0, total_value_list);
	if (last_select_list != NULL)
		HeapFree(heap_handle, 0, last_select_list);
}

inline void set_select_list(unsigned long long item_number, unsigned long long capacity, bool select)
//�������һ����Ʒѡ������б�
{
	unsigned long long position_byte = capacity;
	__int8 position_bit = position_byte % 8;
	position_byte /= 8;
	position_byte += (item_number - 1) * last_select_line_length;//position_byte��ʾ�����õ��ֽڱ�ţ�position_bit��ʾ�����õ�λ��ţ�
	__int8 bit_setter;
	bit_setter = 1 << position_bit;
	if (select)
	{
		last_select_list[position_byte] |= bit_setter;
	}
	else
	{
		bit_setter = ~bit_setter;
		last_select_list[position_byte] &= bit_setter;
	}
}

inline bool get_select_list(unsigned long long item_number, unsigned long long capacity)
{
	unsigned long long position_byte = capacity;
	__int8 position_bit = position_byte % 8;
	position_byte /= 8;
	position_byte += (item_number - 1) * last_select_line_length;//position_byte��ʾ�����õ��ֽڱ�ţ�position_bit��ʾ�����õ�λ��ţ�
	__int8 select = 1 << position_bit;
	return bool(last_select_list[position_byte] & select);
}


inline unsigned long long get_value_position(unsigned long long item_number, unsigned long long capacity)
//��ȡ��Ʒ��item_number���ݻ�capaciy������ܼ�ֵ��һά�����λ��
{
	return (item_number % 2) * (total_capacity + 1) + capacity;
}

static DWORD WINAPI backpack_in_thread(LPVOID lpParameter)//�̺߳���
{
	unsigned long long i_capacity;
	while (true)
	{
		WaitForSingleObject(all_ready_event[((thread_parameter_struct*)lpParameter)->thread_no], INFINITE);//�ȴ����߳�֪ͨ�µ�ѭ����ʼ
		if (all_finish)//����Ѿ�������ϣ����˳�
			break;
		for (i_capacity = ((thread_parameter_struct*)lpParameter)->capacity_min; i_capacity <= ((thread_parameter_struct*)lpParameter)->capacity_max; i_capacity++)
			if (i_item == 1)//�����ǰ��Ʒ��Ϊ1
				if (i_capacity < item_list[0].volume)//����ݻ�С�ڵ�1����Ʒ���������ѡ���1����Ʒ
				{
					total_value_list[get_value_position(1, i_capacity)] = 0;
					set_select_list(1, i_capacity, false);
				}
				else
				{
					total_value_list[get_value_position(1, i_capacity)] = item_list[0].value;
					set_select_list(1, i_capacity, true);
				}
		//�����ǰ��Ʒ������1
			else if (i_capacity < item_list[i_item - 1].volume)//����ݻ�С�ڵ�i_item����Ʒ�������
															  //�ܼ�ֵΪǰi_item-1����Ʒ��i_capacity�ݻ��µ����Ž��ܼ�ֵ����i_item����Ʒ��ѡ��
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);
			}
			else if (total_value_list[get_value_position(i_item - 1, i_capacity)] < total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value)
				//����ݻ������ڵ�i_item����Ʒ��������Ƚϵ�i_item����Ʒ��ѡ��ʱ�͵�i_item����Ʒѡ��ʱ���Ž�ļ�ֵ
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value;
				set_select_list(i_item, i_capacity, true);//��i_item����Ʒѡ��
			}
			else
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);//��i_item����Ʒ��ѡ��
			}
		SetEvent(thread_ready_event[((thread_parameter_struct*)lpParameter)->thread_no]);//ѭ����ϣ�֪ͨ���߳�
	}
	return 0;
}

bool backpack_item_select_multi()//�ö�̬�滮�����������Ķ��̺߳���������̺߳��¼�����ʧ�ܣ��򷵻�false
{
	unsigned int i_thread;
	unsigned long long width_each_thread = total_capacity / thread_number / 8 * 8;
	for (i_thread = 0; i_thread < thread_number; i_thread++)//�����̺߳��¼�
	{
		all_ready_event[i_thread] = CreateEvent(NULL, false, false, NULL);
		if (all_ready_event[i_thread] == NULL)
			return false;//����¼�����ʧ��
		thread_para[i_thread] = { i_thread * width_each_thread, (unsigned long long(i_thread) + 1) * width_each_thread - 1, i_thread };

		thread_ready_event[i_thread] = CreateEvent(NULL, false, false, NULL);
		if (all_ready_event[i_thread] == NULL)
			return false;//����¼�����ʧ��

		thread[i_thread] = CreateThread(NULL, 0, backpack_in_thread, thread_para + i_thread, 0, NULL);
		if (thread[i_thread] == NULL)
			return false;//����̴߳���ʧ��
	}

	thread_para[thread_number - 1].capacity_max = total_capacity;

	//�ö��̼߳���1��total_item_number����Ŀ�����Ž�
	for (i_item = 1; i_item <= total_item_number; i_item++)
	{
		for (i_thread = 0; i_thread < thread_number; i_thread++)
			SetEvent(all_ready_event[i_thread]);//֪ͨ�������߳̿�ʼѭ��

		WaitForMultipleObjects(thread_number, thread_ready_event, true, INFINITE);//�ȴ����߳�ѭ����ϵ�֪ͨ
	}
	all_finish = true;
	for (i_thread = 0; i_thread < thread_number; i_thread++)
	{
		SetEvent(all_ready_event[i_thread]);//֪ͨ�������߳��˳������ر��̺߳��¼�
		CloseHandle(thread_ready_event[i_thread]);
		CloseHandle(all_ready_event[i_thread]);
		CloseHandle(thread[i_thread]);
	}


	return true;
}

void backpack_item_select_single()//�ö�̬�滮�����������ĵ��̺߳���
{
	unsigned long long i_capacity, i_item;//��ǰ������ݻ�����Ʒ����ָʾ��

										  //����1����Ʒ�����Ž�
	for (i_capacity = 0; i_capacity <= total_capacity; i_capacity++)
	{
		if (i_capacity < item_list[0].volume)//����ݻ�С�ڵ�1����Ʒ���������ѡ���1����Ʒ
		{
			total_value_list[get_value_position(1, i_capacity)] = 0;
			set_select_list(1, i_capacity, false);
		}
		else
		{
			total_value_list[get_value_position(1, i_capacity)] = item_list[0].value;
			set_select_list(1, i_capacity, true);
		}
	}
	//����2��total_item_number����Ŀ�����Ž�
	for (i_item = 2; i_item <= total_item_number; i_item++)
	{
		for (i_capacity = 0; i_capacity <= total_capacity; i_capacity++)//�����ݻ�i_capacity��ǰi_item����Ʒ�����Ž�
		{
			if (i_capacity < item_list[i_item - 1].volume)//����ݻ�С�ڵ�i_item����Ʒ�������
														  //�ܼ�ֵΪǰi_item-1����Ʒ��i_capacity�ݻ��µ����Ž��ܼ�ֵ����i_item����Ʒ��ѡ��
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);
			}
			else if (total_value_list[get_value_position(i_item - 1, i_capacity)] < total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value)
				//����ݻ������ڵ�i_item����Ʒ��������Ƚϵ�i_item����Ʒ��ѡ��ʱ�͵�i_item����Ʒѡ��ʱ���Ž�ļ�ֵ
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value;
				set_select_list(i_item, i_capacity, true);//��i_item����Ʒѡ��
			}
			else
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);//��i_item����Ʒ��ѡ��
			}
		}
	}
}
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	MessageBoxA(0, "Caculation start.", "Start", 0);
	DWORD begin_time, end_time;//����ʼʱ�����ֹʱ��
	begin_time = GetTickCount();//������ʼʱ��

	input_handle = CreateFileA("input.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//�������ļ�
	if (input_handle == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(0, "File open error.", "Error", 0);
		return 1;
	}
	input_filesize_low = GetFileSize(input_handle, &input_filesize_high);
	input_filesize = input_filesize_high;
	input_filesize <<= 32;
	input_filesize += input_filesize_low;
	input_map = CreateFileMapping(input_handle, NULL, PAGE_READONLY, input_filesize_high, input_filesize_low, NULL);
	if (input_map == NULL)//�����ڴ�ӳ��
	{
		MessageBoxA(0, "Read error.", "Error", 0);
		return 2;
	}

	input_string = (char*)MapViewOfFile(input_map, FILE_MAP_READ, 0, 0, 0);

	//�������ݻ�����Ʒ����

	if (!read_int(&total_capacity))
	{
		MessageBoxA(0, "Data error.", "Error", 0);
		return 3;
	}
	if (!read_int(&total_item_number))
	{
		MessageBoxA(0, "Data error.", "Error", 0);
		return 3;
	}
	if (total_capacity == 0)
	{
		MessageBoxA(0, "No capacity.", "Error", 0);
		return 3;
	}
	if (total_item_number == 0)
	{
		MessageBoxA(0, "No item.", "Error", 0);
		return 3;
	}

	//�����ڴ�
	heap_handle = GetProcessHeap();
	if (!item_solution_init())
	{
		MessageBoxA(0, "No enough memory.", "Error", 0);
		void item_solution_delete();
		return 4;
	}

	unsigned long long i_item;
	for (i_item = 0; i_item < total_item_number; i_item++)
	{
		unsigned long long volume, value;
		if (!read_int(&volume))
		{
			MessageBoxA(0, "Data error.", "Error", 0);
			return 3;
		}
		if (!read_int(&value))
		{
			MessageBoxA(0, "Data error.", "Error", 0);
			return 3;
		}
		item_list[i_item] = { volume,value };
	}
	if (!UnmapViewOfFile(input_string))
	{
		MessageBoxA(0, "Read error.", "Error", 0);
		return 2;
	}
	CloseHandle(input_handle);
	CloseHandle(input_map);

	if (total_capacity < threshold)	//��ⱳ������
		backpack_item_select_single();
	else
		if (!backpack_item_select_multi())
			backpack_item_select_single();//������߳�ʧ�ܣ����õ��߳����



	//Ԥ����Ҫ������ֽ������41 * total_item_number + 100
	unsigned long long output_estimate_size;
	output_estimate_size = 41 * total_item_number + 100;
	output_string = new char[output_estimate_size];
	if (output_string == NULL)
	{
		MessageBoxA(0, "No enough memory.", "Error", 0);
		void item_solution_delete();
		return 4;
	}

	unsigned long long result_written_size = 0;//���������Ľ�����ַ���
											   //���Ƚ���������ݱ�����output_string�ַ�����
	result_written_size += sprintf_s(output_string, output_estimate_size, "---Best Solution---\r\nTotal Value: %llu\r\n", total_value_list[get_value_position(total_item_number, total_capacity)]);

	//������Ҹ���Ʒ�Ƿ�ѡ��
	unsigned long long current_capacity = total_capacity;
	for (i_item = total_item_number; i_item > 0; i_item--)
	{
		if (final_select_list[i_item - 1] = get_select_list(i_item, current_capacity))
			current_capacity -= item_list[i_item - 1].volume;
	}

	//�������Ʒ�Ƿ�ѡ��
	for (i_item = 1; i_item <= total_item_number; i_item++)
	{
		if (final_select_list[i_item - 1])
			result_written_size += sprintf_s(output_string + result_written_size, output_estimate_size - result_written_size, "Item %llu selected.\r\n", i_item);
		else
			result_written_size += sprintf_s(output_string + result_written_size, output_estimate_size - result_written_size, "Item %llu not selected.\r\n", i_item);
	}

	output_handle = CreateFileA("output.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//������ļ�
	if (output_handle == NULL)
	{
		MessageBoxA(0, "File open error.", "Error", 0);
		return 1;
	}

	DWORD size_written;

	if (result_written_size >= 0xffffffff)//�����д���ֽ�������WriteFile����һ��д����ֽ����ޣ�2GB-1��������ִ�д��
	{
		if (!WriteFile(output_handle, output_string, 0xffffffff, &size_written, NULL))
		{
			MessageBoxA(0, "Write error.", "Error", 0);
			return 2;
		}
		if (size_written != 0xffffffff)
		{
			MessageBoxA(0, "Write error.", "Error", 0);
			return 2;
		}
		result_written_size -= 0xffffffff;
		output_string += 0xffffffff;
	}
	if (!WriteFile(output_handle, output_string, unsigned long(result_written_size), &size_written, NULL))
	{
		MessageBoxA(0, "Write error.", "Error", 0);
		return 2;
	}
	if (size_written != result_written_size)
	{
		MessageBoxA(0, "Write error.", "Error", 0);
		return 2;
	}
	end_time = GetTickCount();//��ȡ������ֹʱ��
	char processing_time_text[50];
	unsigned int processing_time_text_size = sprintf_s(processing_time_text, "Processing Time: %u ms.", end_time - begin_time);
	if (!WriteFile(output_handle, processing_time_text, processing_time_text_size, &size_written, NULL))
	{
		MessageBoxA(0, "Write error.", "Error", 0);
		return 2;
	}
	if (size_written != strlen(processing_time_text))
	{
		MessageBoxA(0, "Write error.", "Error", 0);
		return 2;
	}
	CloseHandle(output_handle);
	void item_solution_delete();
	MessageBoxA(0, "Calculation complete.", "Complete", 0);
	return 0;
}