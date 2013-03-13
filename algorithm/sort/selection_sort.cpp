#include "include/sort.h"

void selection_sort(int *data, int len)
{
    int min = 0;
    int postion = 0; // forget this line
    for (int i = 0; i < len; ++i) 
    {
        min = data[i];
        for (int j = i + 1; j < len; ++j)
        {
            if (data[j] < min)
            {
                min = data[j];
                postion = j; // must record the postion
            }
        }
        data[postion] = data[i];
        data[i] = min;
    }
}
