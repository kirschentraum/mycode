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
                                printf("a��С͵��");
                            if (b == 1)
                                printf("b��С͵��");
                            if (c == 1)
                                printf("c��С͵��");
                            if (d == 1)
                                printf("d��С͵��");
                        }
                    }
                }
    return 1;
}