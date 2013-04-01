#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge_sort(int *a, int len)
{
    int left_min, left_max, right_min, right_max, next;
    int *temp = (int *) malloc(sizeof(int) * len);
    memset(temp, 0, len);

    for (int i = 1; i < len; i *= 2) { // i = 1;
        for (left_min = 0; left_min < len - i; left_min = right_max) {
            next = 0;
            left_max = right_min = left_min + i;
            right_max = right_min + i;

            if (right_max > len)
                right_max = len;

            while (left_min < left_max && right_min < right_max)
                temp[next++] = a[left_min] > a[right_min] ? a[right_min++] : a[left_min++];

            while (left_min < left_max)
                a[--right_min] = a[--left_max];

            while (next > 0)
                a[--right_min] = temp[--next];
        }
    }
    if (temp != NULL)
        free(temp);
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
}
