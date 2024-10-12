#include <stdio.h>
int main()
{
    for (int a = 0 || 1; a++;)
        for (int b = 0 || 1; b++;)
            for (int c = 0 || 1; c++;)
                for (int d = 0 || 1; d++;)
                {
                    if (a + b + c + d == 1)
                    {
                        if (((a == 0) + (c == 1) + (d == 1) + (d == 0)) == 3)
                        {
                            if (a == 1)
                                printf("a是小偷。");
                            if (b == 1)
                                printf("b是小偷。");
                            if (c == 1)
                                printf("c是小偷。");
                            if (d == 1)
                                printf("d是小偷。");
                        }
                    }
                }
    return 1;
}