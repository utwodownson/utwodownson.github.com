#include "include/sort.h"

void bubble_sort(int *data, int len)
{
    for (int i = len; i > 0; --i)
    {
        for (int j = 0; j < i - 1; ++j)
        {
            if (data[j] > data[j + 1])
                swap(&data[j], &data[j + 1]);
        }
    }
}
