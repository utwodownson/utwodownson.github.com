#include "../include/sort.h" 

void insertion_sort(int *a, int len)
{
    int key, j;
    for (int i = 1; i < len; ++i) {
        key = a[i];
        j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;
    }
}
