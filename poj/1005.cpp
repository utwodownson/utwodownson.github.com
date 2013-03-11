#include <stdio.h>

#define PI 3.14

int main()
{
    int num = 0;
    float x = 0;
    float y = 0;
    float sum = 0;
    int year = 0;
    while(scanf("%d", &num) != EOF)
    {
        for (int i = 0; i < num; ++i) 
        {
            scanf("%f%f", &x, &y);
            sum = (x * x + y * y) * PI;
            if ((int)sum % 100 == 0)
                year = sum / 100;
            else
                year  = (sum / 100) + 1;
            printf("Property %d: This property will begin eroding in year %d.\n", i+1, year);
        }
        printf("END OF OUTPUT.\n");
    }    
}
