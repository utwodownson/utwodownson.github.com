#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void merge(int *a, int left, int mid, int right)
{
    int len = right - left + 1;
    int *temp = (int *) malloc(sizeof(int) * len);
    memset(temp, 0, len);

    int k = 0, i = left, j = mid + 1;

    while (i <= mid && j <= right)
        temp[k++] = a[i] > a[j] ? a[j++] : a[i++];
    while (i <= mid)
        temp[k++] = a[i++];
    while (j <= right)
        temp[k++] = a[j++];

    for (int p = 0; p < k; ++p)
        a[p + left] = temp[p];

    if (temp != NULL)
        free(temp);
}

void merge_sort(int *a, int left, int right)
{
    if (left < right) {
        int mid = (left & right) + ((left ^ right) >> 1);
        merge_sort(a, left, mid);
        merge_sort(a, mid + 1, right);
        merge(a, left, mid, right);
    }
}

int main()
{
    int n = 0;
    int a[100];
    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);
    merge_sort(a, 0, n - 1);
    for (int i = 0; i < n; ++i)
        printf("%d ", a[i]);
}
