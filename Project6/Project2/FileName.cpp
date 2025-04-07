#include<iostream>

int mini = 10000;
const int sum = 1800;

int lcm(int a, int b, int c, int d, int e)
{
	int g = e;
	while (true)
	{
		if ((g % a == 0 && g % b == 0 && g % c == 0 && g % d == 0 && g % e == 0)||(g>=mini))
		{
			return g;
		}
		g+=e;
	}
}

int main()
{
	using namespace std;
	int a, b, c, d, e, g, min_a, min_b, min_c, min_d, min_e;
	for(a=1;a*5<sum; a++)
		for(b=a+1; a+b*4<sum; b++)
			for(c=b+1; a+b+c*3<sum; c++)
				for (d = c + 1; a+b+c+d*2<sum; d++)
				{
					e = sum - a - b - c - d;
					if (e > d)
					{
						g=lcm(a, b, c, d, e);
						if (g < mini)
						{
							mini = g;
							min_a = a;
							min_b = b;
							min_c = c;
							min_d = d;
							min_e = e;
						}
					}
				}
	cout << "lcm_min=" << mini << endl;
	cout << min_a << " " << min_b << " " <<min_c <<" " << min_d << " " << min_e;
}