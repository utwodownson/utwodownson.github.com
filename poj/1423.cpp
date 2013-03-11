#include <stdio.h>
#include <math.h>

#define PI 3.1415926

int main()
{
    int n = 0;
    int num = 0;
    int temp = 0;
    while (scanf("%d", &n) != EOF)
    {
        for (int i = 0; i < n; ++i)
        {
            scanf("%d", &num);
            if (num == 1)
            {
                printf("%d\n", num);
                continue;
            }
            temp = ceil((num * log(num) - num + log(2 * num * PI) / 2) / log(10));
            printf("%d\n", temp);
        }
    }
}
