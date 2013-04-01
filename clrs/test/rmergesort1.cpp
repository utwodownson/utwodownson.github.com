#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void merge_sort(int *a, int len)
{
    int left_min, left_max, right_min, right_max;
    int *temp = (int *) malloc(sizeof(int) * len);

    for (int i = 1; i < len; i *= 2) {
        for (int left_min = 0; left_min < len - i; left_min = right_max) {
            left_max = right_min = left_min + i;
            right_max = right_min + i;

            int next = 0;
            while (left_min < left_max && right_min < right_max)
                temp[next++] = a[left_min] > a[]
        }
    }
}
