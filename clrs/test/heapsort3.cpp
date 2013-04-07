#include <stdio.h>

void swap(int *a, int *b)
{
    *a = *a ^ *b;
    *b = *a ^ *b;
    *a = *a ^ *b;
}

void heapdown(int *a, int i, int len)
{
    int child, key;
    for (key = a[i]; 2 * i + 1 < len; i = child) {
        child = 2 * i + 1;
        while (child + 1 < len && a[child + 1] > a[child]) ++child;
        if (key < a[child]) a[i] = a[child];
        else                break;
        a[child] = key;
    }
}

void heapsort(int *a, int len)
{
    for (int i = len / 2 - 1; i >= 0; --i)
        heapdown(a, i, len);
    for (int i = len - 1; i > 0; --i) {
        swap(&a[0], &a[i]);
        heapdown(a, 0, i);
    }
}

int main()
{
    int a[100] = { 5, 3, 1, 2, 3, 6, 7, 4, 3 };
    int len = 9;
    heapsort(a, len);
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
