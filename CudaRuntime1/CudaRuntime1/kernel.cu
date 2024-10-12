
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

__global__ void add(int a, int b, int* c) 
{
	*c = a + b;
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	int c;
	int* dev_c;
	dev_c=new int(1);
	int handle=cudaMalloc((void**)&dev_c, sizeof(int));
	add <<<2, 2>>> (2, 7, dev_c);
	cudaMemcpy(&c,	dev_c, sizeof(int),	cudaMemcpyDeviceToHost);
	char result[11];
	itoa(c, result, 10);
	MessageBox(NULL, TEXT(result), "result", MB_OK);
	cudaFree(dev_c);

	size_t free_byte;
	size_t total_byte;

	cudaError_t cuda_status = cudaMemGetInfo(&free_byte, &total_byte);
	itoa(free_byte, result, 10);
	
	MessageBox(NULL, TEXT(result), "result", MB_OK);


	return 0;

}