#include<cstdio>

void func(int a, int b)
{
	printf("%d\n%d\n", a, b);
}

int main()
{
	int c = 25;
	func(++c, c);
	return 0;
}