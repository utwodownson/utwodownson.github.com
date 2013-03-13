#include "include/sort.h"

void shell_sort(int *data, int len)
{
    const int n = 5;
    int i, j, temp; 
    int gap = 0;
    while (gap<=n)
    {
        gap = gap * 3 + 1;
    } 
    while (gap > 0) 
    {
        for ( i = gap; i < n; i++ )
        {
            j = i - gap;
            temp = data[i];             
            while (( j >= 0 ) && ( data[j] > temp ))
            {
                data[j + gap] = data[j];
                j = j - gap;
            }
            data[j + gap] = temp;
        }
        gap = ( gap - 1 ) / 3;
    }    
}
