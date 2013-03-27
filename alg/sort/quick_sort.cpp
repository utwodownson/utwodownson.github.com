#include "include/sort.h"

int position(int *data, int low, int high)
{
    int key = data[low];
    int i = low;
    int j = high;
    while (i < j) 
    {
        while (i < j && data[j] >= key) --j;
        if (i < j)  data[i++] = data[j];
        while (i < j && data[i] <= key) ++i;
        if (i < j) data[j--] = data[i]; 
    }
    data[i] = key;
    return (i);
}

void quick_sort(int *data, int low, int high)
{
    if (low < high) {
        int flag = position(data, low, high);
        quick_sort(data, low, flag - 1); // flag - 1 because the flag is in the row
        quick_sort(data, flag + 1, high);
    }
}
