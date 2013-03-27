// forget the alg
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void merge_sort(int *a, int len)
{
    int left_min, left_max, right_min, right_max;
    int *buf = (int *) malloc(sizeof(int) * len);
    int k;
    
    for (int i = 1; i < len; i *= 2) {
        for (int left_min = 0; left_min < len - i; left_min = right_max) {
            right_min = left_max = left_min + i;
            right_max = right_min + i;

            if (right_max > len)
                right_max = len;

            k = 0;   
            while (left_min < left_max && right_min < right_max)
                buf[k++] = a[left_min] < a[right_min] ? a[left_min++] : a[right_min++];

            while (left_min < left_max)
                a[--right_min] = a[--left_max];
            while (k > 0)
                a[--right_min] = buf[--k];
        }
    }
    if (buf != NULL)
        free(buf);
}

int main()
{
    int n = 0;
    int a[10];
    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);
    merge_sort(a, n);
    for (int i = 0; i < n; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
