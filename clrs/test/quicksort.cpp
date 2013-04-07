#include <stdio.h>

int position(int *a, int low, int high)
{
    int i = low;
    int j = high;
    int key = a[low];
    while (i < j) {
        while (i < j && key <= a[j]) --j;
        if (i < j) a[i++] = a[j];
        while (i < j && a[i] < key) ++i;
        if (i < j) a[j--] = a[i];
    }
    a[i] = key;
    return i;
}

void quick_sort(int *a, int low, int high)
{
    if (low < high) {
        int flag = position(a, low, high);
        quick_sort(a, low, flag - 1);
        quick_sort(a, flag + 1, high);
    }
}

int main()
{
    int a[100] = { 5, 6, 2, 1, 4, 0 ,3, 1 };
    int len = 8;
    quick_sort(a, 0, len - 1);
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
