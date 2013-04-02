#include "../include/sort.h"

void heapdown(int *a, int i, int len)
{
    int child, temp;
    for (temp = a[i]; 2 * i + 1 < len; i = child) {
        child = 2 * i + 1;
        if (child < len - 1 && a[child + 1] > a[child])
            ++child;
        if (temp < a[child]) 
            a[i] = a[child];
        else
            break;
        a[child] = temp;
    }
}

void heapsort(int *a, int len)
{
    for (int i = len / 2 - 1; i >= 0; --i)
        heapdown(a, i, len);

    for (int i = len - 1; i > 0; --i) {
        swap(&a[0], &a[i]);
        heapdown(a, 0, i);
    }
}
