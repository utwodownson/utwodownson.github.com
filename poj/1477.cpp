#include <stdio.h>

int main()
{
    int n = 0;
    int sum = 0;
    int final = 0;
    int result = 0;
    int time = 0;
    int a[50];
    int i = 0;

    while((scanf("%d", &n) != EOF) && (n != 0)) 
    {   
        sum = 0;
        result = 0;
        time ++; 
        for (i = 0; i < n; ++i)
        {   
            scanf("%d", &a[i]);        
            sum += a[i];
        }                    
        final = sum / n;
        for (i = 0; i < n; ++i)
        {   
            if (a[i] < final)
                result += (final - a[i]);        
        }   
        printf("Set #%d\n", time);
        printf("The minimum number of moves is %d.\n", result);
        printf("\n");
    }        
}
