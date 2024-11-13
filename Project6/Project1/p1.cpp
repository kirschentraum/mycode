#include<iostream>
const int n = 6;

void print(int a[n][n], int sum)
{
	std::cout << "Result no. " << sum << ":" << std::endl;
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
			std::cout << a[i][j] << ' ';
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void find(int a[n][n], int cur_i, int cur_j, int& sum)
{
	for (int k = 0; k < n; k++)
	{
		bool sign = true;
		for (int p = 0; p < cur_i; p++)
			if (a[p][cur_j] == k)
				sign = false;
		for (int p = 0; p < cur_j; p++)
			if (a[cur_i][p] == k)
				sign = false;
		if (sign)
		{
			a[cur_i][cur_j] = k;
			if (cur_i == n - 1 && cur_j == n - 1)
			{
				sum++;
				print(a, sum);
			}
			else if (cur_j == n - 1)
				find(a, cur_i + 1, 1, sum);
			else
				find(a, cur_i, cur_j + 1, sum);
		}
	}
}

int main()
{
	int sum = 0;
	int a[n][n];
	for (int i = 0; i < n; i++)
	{
		a[0][i] = i;
		a[i][0] = i;
	}
	find(a, 1, 1, sum);
	std::cout <<"Total:"<< sum << std::endl;
	return 0;
}