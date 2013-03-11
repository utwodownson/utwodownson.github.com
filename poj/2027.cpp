#include <stdio.h>

int main()
{
    int number = 0;
    int a, b;
    while (scanf("%d", &number) != EOF)
    {
        for (int i = 0; i < number; ++i)
        {
            scanf("%d%d", &a, &b);
            if (a < b)
                printf("NO BRAINS\n");
            else
                printf("MMM BRAINS\n");
        }
    }
}
