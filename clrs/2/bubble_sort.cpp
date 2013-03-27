#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void swap(int *a, int *b)
{
    *a = *a ^ *b;
    *b = *a ^ *b;
    *a = *a ^ *b;
}

void bubble_sort(int *a, int len)
{
    for (int i = len; i > 0; --i) {
        for (int j = 0; j < i - 1; ++j) {
            if (a[j] > a[j + 1])
                swap(&a[j], &a[j + 1]);
        }
    }
}

int main()
{
    int n = 0;
    int a[10];
    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);
    bubble_sort(a, n);
    for (int i = 0; i < n; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
