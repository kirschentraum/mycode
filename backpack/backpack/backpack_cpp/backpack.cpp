#include <Windows.h>
#include <stdio.h>
//背包问题，运行于x64平台
//输入文件名：input.txt
//输入格式：
//背包容积 物品数量 物品1的体积 物品1的价值 [[物品2的体积 物品2的价值] ...]
//输出文件名：output.txt

const unsigned int thread_number = 8;//线程数
const unsigned long long threshold = 10000;//开启多线程的背包容积阈值

HANDLE thread[thread_number];//线程句柄
HANDLE thread_ready_event[thread_number];//子线程循环计算完毕的事件句柄
HANDLE all_ready_event[thread_number];//所有子线程循环计算完毕，开始新一轮循环的事件句柄

char* input_string, * output_string;;//指向输入文件内容和输出文件内容的字符串
unsigned long long total_capacity, total_item_number;//背包容积，物品数量
HANDLE input_handle, output_handle;//输入文件句柄和输出文件句柄
unsigned long input_filesize_low, input_filesize_high;//输入文件高位和地位大小
unsigned long long input_filesize;//输入和输出文件总大小
HANDLE input_map;//输入文件的内存映射句柄
HANDLE heap_handle;//堆句柄

unsigned long long i_item;//当前计算的物品数
bool all_finish;//表示所有计算任务已经完成

struct thread_parameter_struct
{
	unsigned long long capacity_min, capacity_max;
	unsigned int thread_no;
};

thread_parameter_struct thread_para[thread_number];//各线程参数

struct item//每个物品的信息
{
	unsigned long long volume, value;
};

item* item_list;//物品信息列表
unsigned long long* total_value_list;//总价值列表
__int8* last_select_list;//最后一个物品选择情况列表
bool* final_select_list;//最终解答的全部物品选择情况列表
unsigned long long last_select_line_length;//最后一个物品选择情况列表的每行长度

inline bool read_int(unsigned long long* result)//从长度为input_filesize的字符串input_string读入一个64位无符号整数的函数，返回到result
												//如果读不到字符串结尾仍然读不到任何数字，则返回false，否则返回true
{
	static unsigned long long string_pointer{ 0 };//当前读取的字符位置
	unsigned long long value = 0;
	do//读第一个数字的循环
	{
		if (string_pointer >= input_filesize)//如果已经读到结尾仍未读到数字，返回错误
			return false;
		value = unsigned long long(input_string[string_pointer]) - 48;
		string_pointer++;
	} while (value > 9);

	if (string_pointer >= input_filesize)
	{
		*result = value;
		return true;
	}
	while (input_string[string_pointer] >= '0' && input_string[string_pointer] <= '9')//读后面数字的循环
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

inline bool item_solution_init()//物品信息列表、总价值列表和最后一个物品选择情况列表初始化，如果内存不足则返回false
{
	item_list = (item*)HeapAlloc(heap_handle, 0, (total_item_number)*sizeof(item));
	if (item_list == NULL)
		return false;
	final_select_list = (bool*)HeapAlloc(heap_handle, 0, (total_item_number)*sizeof(bool));
	if (final_select_list == NULL)
		return false;
	total_value_list = (unsigned long long *)HeapAlloc(heap_handle, 0, 2 * (total_capacity + 1) * sizeof(unsigned long long));//最优解总价值只保留最近计算的2行，以节省内存
	if (total_value_list == NULL)
		return false;
	last_select_line_length = total_capacity / 64 * 8 + 8;
	last_select_list = (char*)HeapAlloc(heap_handle, 0, last_select_line_length * total_item_number);//计算最后一个物品选择情况列表所需的字节数
	if (last_select_list == NULL)
		return false;
	return true;
}

inline void item_solution_delete()//释放已分配的内存
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
//设置最后一个物品选择情况列表
{
	unsigned long long position_byte = capacity;
	__int8 position_bit = position_byte % 8;
	position_byte /= 8;
	position_byte += (item_number - 1) * last_select_line_length;//position_byte表示待设置的字节编号，position_bit表示待设置的位编号；
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
	position_byte += (item_number - 1) * last_select_line_length;//position_byte表示待设置的字节编号，position_bit表示待设置的位编号；
	__int8 select = 1 << position_bit;
	return bool(last_select_list[position_byte] & select);
}


inline unsigned long long get_value_position(unsigned long long item_number, unsigned long long capacity)
//获取物品数item_number和容积capaciy的最大总价值在一维数组的位置
{
	return (item_number % 2) * (total_capacity + 1) + capacity;
}

static DWORD WINAPI backpack_in_thread(LPVOID lpParameter)//线程函数
{
	unsigned long long i_capacity;
	while (true)
	{
		WaitForSingleObject(all_ready_event[((thread_parameter_struct*)lpParameter)->thread_no], INFINITE);//等待主线程通知新的循环开始
		if (all_finish)//如果已经计算完毕，则退出
			break;
		for (i_capacity = ((thread_parameter_struct*)lpParameter)->capacity_min; i_capacity <= ((thread_parameter_struct*)lpParameter)->capacity_max; i_capacity++)
			if (i_item == 1)//如果当前物品数为1
				if (i_capacity < item_list[0].volume)//如果容积小于第1个物品的体积，不选择第1个物品
				{
					total_value_list[get_value_position(1, i_capacity)] = 0;
					set_select_list(1, i_capacity, false);
				}
				else
				{
					total_value_list[get_value_position(1, i_capacity)] = item_list[0].value;
					set_select_list(1, i_capacity, true);
				}
		//如果当前物品数大于1
			else if (i_capacity < item_list[i_item - 1].volume)//如果容积小于第i_item个物品的体积，
															  //总价值为前i_item-1个物品在i_capacity容积下的最优解总价值，第i_item个物品不选择
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);
			}
			else if (total_value_list[get_value_position(i_item - 1, i_capacity)] < total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value)
				//如果容积不大于第i_item个物品的体积，比较第i_item个物品不选择时和第i_item个物品选择时最优解的价值
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value;
				set_select_list(i_item, i_capacity, true);//第i_item个物品选择
			}
			else
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);//第i_item个物品不选择
			}
		SetEvent(thread_ready_event[((thread_parameter_struct*)lpParameter)->thread_no]);//循环完毕，通知主线程
	}
	return 0;
}

bool backpack_item_select_multi()//用动态规划解决背包问题的多线程函数，如果线程和事件创建失败，则返回false
{
	unsigned int i_thread;
	unsigned long long width_each_thread = total_capacity / thread_number / 8 * 8;
	for (i_thread = 0; i_thread < thread_number; i_thread++)//创建线程和事件
	{
		all_ready_event[i_thread] = CreateEvent(NULL, false, false, NULL);
		if (all_ready_event[i_thread] == NULL)
			return false;//如果事件创建失败
		thread_para[i_thread] = { i_thread * width_each_thread, (unsigned long long(i_thread) + 1) * width_each_thread - 1, i_thread };

		thread_ready_event[i_thread] = CreateEvent(NULL, false, false, NULL);
		if (all_ready_event[i_thread] == NULL)
			return false;//如果事件创建失败

		thread[i_thread] = CreateThread(NULL, 0, backpack_in_thread, thread_para + i_thread, 0, NULL);
		if (thread[i_thread] == NULL)
			return false;//如果线程创建失败
	}

	thread_para[thread_number - 1].capacity_max = total_capacity;

	//用多线程计算1至total_item_number个项目的最优解
	for (i_item = 1; i_item <= total_item_number; i_item++)
	{
		for (i_thread = 0; i_thread < thread_number; i_thread++)
			SetEvent(all_ready_event[i_thread]);//通知所有子线程开始循环

		WaitForMultipleObjects(thread_number, thread_ready_event, true, INFINITE);//等待子线程循环完毕的通知
	}
	all_finish = true;
	for (i_thread = 0; i_thread < thread_number; i_thread++)
	{
		SetEvent(all_ready_event[i_thread]);//通知所有子线程退出，并关闭线程和事件
		CloseHandle(thread_ready_event[i_thread]);
		CloseHandle(all_ready_event[i_thread]);
		CloseHandle(thread[i_thread]);
	}


	return true;
}

void backpack_item_select_single()//用动态规划解决背包问题的单线程函数
{
	unsigned long long i_capacity, i_item;//当前计算的容积和物品数量指示器

										  //计算1个物品的最优解
	for (i_capacity = 0; i_capacity <= total_capacity; i_capacity++)
	{
		if (i_capacity < item_list[0].volume)//如果容积小于第1个物品的体积，不选择第1个物品
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
	//计算2至total_item_number个项目的最优解
	for (i_item = 2; i_item <= total_item_number; i_item++)
	{
		for (i_capacity = 0; i_capacity <= total_capacity; i_capacity++)//计算容积i_capacity，前i_item个物品的最优解
		{
			if (i_capacity < item_list[i_item - 1].volume)//如果容积小于第i_item个物品的体积，
														  //总价值为前i_item-1个物品在i_capacity容积下的最优解总价值，第i_item个物品不选择
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);
			}
			else if (total_value_list[get_value_position(i_item - 1, i_capacity)] < total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value)
				//如果容积不大于第i_item个物品的体积，比较第i_item个物品不选择时和第i_item个物品选择时最优解的价值
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity - item_list[i_item - 1].volume)] + item_list[i_item - 1].value;
				set_select_list(i_item, i_capacity, true);//第i_item个物品选择
			}
			else
			{
				total_value_list[get_value_position(i_item, i_capacity)] = total_value_list[get_value_position(i_item - 1, i_capacity)];
				set_select_list(i_item, i_capacity, false);//第i_item个物品不选择
			}
		}
	}
}
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	MessageBoxA(0, "Caculation start.", "Start", 0);
	DWORD begin_time, end_time;//程序开始时间和终止时间
	begin_time = GetTickCount();//读程序开始时间

	input_handle = CreateFileA("input.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//打开输入文件
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
	if (input_map == NULL)//创建内存映射
	{
		MessageBoxA(0, "Read error.", "Error", 0);
		return 2;
	}

	input_string = (char*)MapViewOfFile(input_map, FILE_MAP_READ, 0, 0, 0);

	//读背包容积和物品数量

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

	//分配内存
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

	if (total_capacity < threshold)	//求解背包问题
		backpack_item_select_single();
	else
		if (!backpack_item_select_multi())
			backpack_item_select_single();//如果多线程失败，仍用单线程求解



	//预估需要输出的字节数最多41 * total_item_number + 100
	unsigned long long output_estimate_size;
	output_estimate_size = 41 * total_item_number + 100;
	output_string = new char[output_estimate_size];
	if (output_string == NULL)
	{
		MessageBoxA(0, "No enough memory.", "Error", 0);
		void item_solution_delete();
		return 4;
	}

	unsigned long long result_written_size = 0;//计算待输出的结果总字符数
											   //首先将待输出内容保存在output_string字符串中
	result_written_size += sprintf_s(output_string, output_estimate_size, "---Best Solution---\r\nTotal Value: %llu\r\n", total_value_list[get_value_position(total_item_number, total_capacity)]);

	//倒序查找各物品是否选择
	unsigned long long current_capacity = total_capacity;
	for (i_item = total_item_number; i_item > 0; i_item--)
	{
		if (final_select_list[i_item - 1] = get_select_list(i_item, current_capacity))
			current_capacity -= item_list[i_item - 1].volume;
	}

	//输出各物品是否选择
	for (i_item = 1; i_item <= total_item_number; i_item++)
	{
		if (final_select_list[i_item - 1])
			result_written_size += sprintf_s(output_string + result_written_size, output_estimate_size - result_written_size, "Item %llu selected.\r\n", i_item);
		else
			result_written_size += sprintf_s(output_string + result_written_size, output_estimate_size - result_written_size, "Item %llu not selected.\r\n", i_item);
	}

	output_handle = CreateFileA("output.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//打开输出文件
	if (output_handle == NULL)
	{
		MessageBoxA(0, "File open error.", "Error", 0);
		return 1;
	}

	DWORD size_written;

	if (result_written_size >= 0xffffffff)//如果待写入字节数超过WriteFile函数一次写入的字节上限（2GB-1），则需分次写入
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
	end_time = GetTickCount();//获取程序终止时间
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