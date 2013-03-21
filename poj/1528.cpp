// 28 is a bad case

#include <stdio.h>

void format(int n)
{
    if (n > 0 && n < 10)
        printf("    ");
    else if (n > 9 && n < 100)
        printf("   ");
    else if (n > 99 && n < 1000)
        printf("  ");
    else if (n > 999 && n < 10000) 
        printf(" ");
    else
        printf("");

    printf("%d", n);
}

void fun(int n)
{
    int sum = 0;

    format(n);

    for (int i = 1; i < n; ++i)
    {
        if (n % i == 0) // not use number % i
            sum += i;
        if (sum > n)
            break;
    }
    if (sum == n)
        printf("  PERFECT\n");
    else if (sum < n) 
        printf("  DEFICIENT\n");
    else 
        printf("  ABUNDANT\n");
}

int main()
{
    int n = 0;
    printf("PERFECTION OUTPUT\n");
    while (scanf("%d", &n) != EOF && n != 0)
        fun(n);
    printf("END OF OUTPUT\n");
}
