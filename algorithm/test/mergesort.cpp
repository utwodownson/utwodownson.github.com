// 10min
// two times
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void merge(int *a, int left, int mid, int right)
{
    int *temp;
    int len = right - left + 1;
    temp = (int *) malloc(sizeof(int) * len);
    memset(temp, 0, sizeof(int) * len);

    int i = left, j = mid;
    int k = 0;
    while (i < mid - 1 && j < right)
        temp[k++] = a[i] < a[j] ? a[i++] : a[j++]; 

    while (i < mid - 1)
        temp[k++] = a[i++];

    while (j < right)
        temp[k++] = a[j++];

    for (int p = 0; p < k; ++p)
        a[left + p] = temp[p];
}

void merge_sort(int *a, int left, int right)
{
    int mid = 0;
    if (left < right) {
        mid = left & right + ((left ^ right) >> 1);
        merge_sort(a, left, mid - 1);
        merge_sort(a, mid, right);
        merge(a, left, mid, right); 
    }
}

int main()
{
    int n = 0;
    int a[10];
    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);
    merge_sort(a, 0, n - 1);
    for (int i = 0; i < n; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
