#include<stdio.h>
#include<stdlib.h>
char *c1, *c2, mydata, nextdata;


extern void FlagFunction();

int main()
{
	c1= (char*)malloc(0xA0000000);
	c2= (char*)malloc(0xA0000000);
	mydata = c1[0];
	nextdata = c2[0x80000000];
	FlagFunction();
	return 0;
}