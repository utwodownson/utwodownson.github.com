#include <stdio.h>

int main()
{
    int n = 0;
    int data, num;
    int sum = 0;
    while (scanf("%d", &n) != EOF)
    {
        for (int i = 0; i < n; ++i)
        {
            sum = 0;
            scanf("%d", &num);
            for (int j = 0; j < num; ++j) 
            {
                scanf("%d", &data);
                sum += data;
            }
            printf("%d\n", sum - num + 1);
        }
    }
}
