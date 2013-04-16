#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge(int *a, int left, int mid, int right)
{
    int len = right - left + 1;
    int *buff = (int *) malloc(sizeof(int) * len);
    // donnot forget buff is NULL
    if (buff == NULL)
        return;

    memset(buff, 0, len);
    int i = left;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= right)
        buff[k++] = a[i] <= a[j] ? a[i++] : a[j++]; // this is better
    while (i <= mid)
        buff[k++] = a[i++];
    while (j <= right)
        buff[k++] = a[j++];

    for (int p = 0; p < k; ++p)
        a[left + p] = buff[p];

    if (buff != NULL)
        free(buff); // donnot forget free
}

// recursion
void merge_sort(int *a, int left, int right)
{
    int mid;
    if (left < right) // not left <= right;
    {
        // mid = (left & right) + ((left ^ right) >> 1); is OK 
        mid = left + ((right - left) >> 1);  // must have () outside the >>1
        merge_sort(a, left, mid);
        merge_sort(a, mid + 1, right);
        merge(a, left, mid, right);
    }
}

// un recursion
void merge_sort_un(int *a, int len)
{
    int *buff = (int *) malloc(sizeof(int) * len);
    if (buff == NULL)
        return;

    int left_min, left_max, right_min, right_max, next;
    for (int i = 1; i < len; i *= 2)
    {
        for (left_min = 0; left_min < len - i; left_min = right_max)
        {
            left_max = right_min = left_min + i;
            right_max = right_min + i;

            // donnot forget 
            if (right_max > len)
                right_max = len;

            next = 0;
            while (left_min < left_max && right_min < right_max)
                buff[next++] = a[left_min] <= a[right_min] ? a[left_min++] : a[right_min++];

            while (left_min < left_max)
                a[--right_min] = a[--left_max];

            while (next > 0) // not next >= 0 because of --next
                a[--right_min] = buff[--next];
        }
    }

    if (buff != NULL)
        free(buff);
}

int main()
{
    int a[10] = { 5, 2, 7, 1, 0, 3, 10, 9, 8 };
    int b[10] = { 5, 2, 7, 1, 0, 3, 10, 9, 8 };
    int len = 9;
    printf("recursion merge sort\t:\t");
    merge_sort(a, 0, len - 1);
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");

    printf("unrecursion merge sort\t:\t");
    merge_sort_un(a, len);
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
