#include "include/sort.h"

void shift_up(int a[], int i)
{
    for (int j = (i - 1) / 2; (j >= 0) && (i >= 0) && a[i] > a[j]; i = j, j = (i - 1) / 2)
        swap(&a[i], &a[j]);
}

void shift_down(int a[], int i, int n)
{
    int j = i * 2 + 1;
    while (j < n)  
    {
        if ((j + 1) && (a[j] < a[j + 1]))
            ++j;
        if (a[i] > a[j])
            break;
        
        swap(&a[i], &a[j]); 
        i = j;
        j = i * 2 + 1;
    }
}

void build_max_heap(int a[], int n)
{
    for (int i = n / 2 - 1; i >= 0; ++i) 
        shift_down(a, i, n);
}

void heapsort(int a[], int n)
{
    for (int i = n - 1; i >= 1; ++i)
    {
        swap(&a[i], &a[0]);
        shift_down(a, 0, i);
    }
}
