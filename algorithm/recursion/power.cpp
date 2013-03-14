#include <stdio.h>

long long power(long long number, int m)
{
    if (m == 1)
        return number;
    else if (m == 2)
        return number * number;
    else
        return power(number, m / 2) * power(number, (m + 1) / 2);
}

int main()
{
    long long num = 0;
    int m = 0;
    printf("the power's number is:\n");
    scanf("%lld", &num);
    printf("the m is:\n");
    scanf("%d", &m);
    printf("the power is : %lld\n", power(num, m));
}
