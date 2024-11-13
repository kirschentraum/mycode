#include<iostream>

void copy(int* sor, int* dest)
{
	for (int i = 0; i < 9; i++)
		dest[i] = sor[i];
}

int main()
{
	int max[] = {9, 1, 1, 2, 1, 1, 1, 1,1};
	int fee[] = {5002, 6009, 1596, 5000, 900, 6000,1000, 600, 7006};
	int result[9], a[9];
	int min=0;
	for(a[0]=0;a[0]<=max[0];a[0]++)
		for (a[1] = 0; a[1] <= max[1]; a[1]++)
			for (a[2] = 0; a[2] <= max[2]; a[2]++)
				for (a[3] = 0; a[3] <= max[3]; a[3]++)
					for (a[4] = 0; a[4] <= max[4]; a[4]++)
						for (a[5] = 0; a[5] <= max[5]; a[5]++)
							for (a[6] = 0; a[6] <= max[6]; a[6]++)
								for (a[7] = 0; a[7] <= max[7]; a[7]++)
									for (a[8] = 0; a[8] <= max[8]; a[8]++)
									{
										if(a[0]+a[1]+a[2]==9)
											if (a[3] + a[4] + a[5] == 2)
												if (a[6] + a[7] + a[8] == 1)
													if (a[0] + a[3] + a[6] == 10)
														if (a[1] + a[4] + a[5] == 1)
															if (a[2] + a[5] + a[8] == 1)
															{
																int sum = 0;
																for (int i = 0; i < 9; i++)
																	sum += a[i] * fee[i];
																if (sum > min)
																{
																	min = sum;
																	copy(a, result);
																}
															}

									}
	std::cout << "min=" << min << std::endl;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
			std::cout << result[i*3+j] << ' ';
		std::cout << std::endl;
	}
	return 0;
}