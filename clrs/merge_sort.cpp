#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge(int *a, int left, int mid, int right)
{
    int len = right - left + 1;
    int *buff = (int *) malloc(sizeof(int) * len);
    if (buff == NULL)
        return;
    
    int i = left;
    int j = mid + 1;
    int k = 0;
    while (i <= mid && j <= right)
        buff[k++] = a[i] <= a[j] ? a[i++] : a[j++];
    while (i <= mid)
        buff[k++] = a[i++];
    while (j <= right)
        buff[k++] = a[j++];
    for (int p = 0; p < k; ++p)
        a[left + p] = buff[p];
    
    if (buff != NULL) 
        free(buff);
}

void merge_sort_recursion(int *a, int left, int right)
{
    if (left < right) // left = right ?
    {
        int mid = left + (right - left) / 2; 
        merge_sort_recursion(a, left, mid);
        merge_sort_recursion(a, mid + 1, right);
        merge(a, left, mid, right);
    }
}

void merge_sort_unrecursion(int *a, int len)
{
    int *buff = (int *) malloc(sizeof(int) * len);
    if (buff == NULL)
        return;

    int left_min, left_max, right_min, right_max;
    int next;

    for (int i = 1; i < len; i *= 2) // i *= 2; i = 1;
    {
        for (left_min = 0; left_min < len - i; left_min = right_max)
        {
            left_max = right_min = left_min + i;
            right_max = right_min + i;
            if (right_max > len)
                right_max = len;

            next = 0;
            while (left_min < left_max && right_min < right_max)
                buff[next++] = a[left_min] <= a[right_min] ? a[left_min++] : a[right_min++];
            while (left_min < left_max)
                a[--right_min] = a[--left_max];
            while (next > 0) // next = 0 ? because of --next 
                a[--right_min] = buff[--next];
        }
    }
    if (buff != NULL)
        free(buff);
}

int main()
{
    int a[10] = { 5, 10, 1, 1, 0, 4, 8, 9, 2, 1 };
    int b[10] = { 5, 10, 1, 1, 0, 4, 8, 9, 2, 1 };
    int len = 10;

    // the algorithm
    printf("recursion merge sort : ");
    merge_sort_recursion(a, 0, len - 1);

    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");

    // the algorithm
    printf("unrecursion merge sort : ");
    merge_sort_unrecursion(b, len);

    for (int i = 0; i < len; ++i)
        printf("%d ", b[i]);
    printf("\n");
}
