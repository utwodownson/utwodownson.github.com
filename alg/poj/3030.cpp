#include <stdio.h>

int main()
{
    int number = 0;
    int a, b, c;
    while (scanf("%d", &number) != EOF)
    {
        for (int i = 0; i < number; ++i)
        {
            scanf("%d%d%d", &a, &b, &c);
            if ((b - c) > a)
                printf("advertise\n");
            else if ((b - c) == a)
                printf("does not matter\n");
            else
                printf("do not advertise\n");
        }
    }
}
