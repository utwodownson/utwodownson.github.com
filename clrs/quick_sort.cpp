#include <stdio.h>

int position(int *a, int left, int right)
{
    int key = a[left];
    int i = left, j = right;
    while (i < j) // i != j 
    {
        while (i < j && key <= a[j]) --j;
        if (i < j) a[i++] = a[j];
        while (i < j && a[i] <= key) ++i;
        if (i < j) a[j--] = a[i];
    }
    a[i] = key;
    return i;
}

void quick_sort(int *a, int left, int right)
{
    int mid;
    if (left < right)
    {
        mid = position(a, left, right);
        quick_sort(a, left, mid - 1); // mid - 1 because of the mid is on its position
        quick_sort(a, mid + 1, right);
    }
}

int main()
{
    int a[10] = { 5, 9, 0, 1, 1, 4, 9, 2, 3, 8 };
    int len = 10;
    quick_sort(a, 0, len - 1);
    printf("quick sort : ");
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
