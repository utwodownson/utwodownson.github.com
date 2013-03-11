#include <stdio.h>

int main()
{
    int a = 0;
    double num = 0;
    double sum = 0;
    while (scanf("%lf", &num) != EOF)
    { 
        sum += num; 
        if (a == 11)
        {
            printf("$%.2f\n", sum / 12);
            a = 0;
            sum = 0;
        }
        else
            a++;  
    }
}
