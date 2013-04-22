#include <stdio.h>

void swap(int *a, int *b)
{
    *a = *a ^ *b;
    *b = *a ^ *b;
    *a = *a ^ *b;
}

void heap_down(int *a, int i, int len)
{
    int key, child;
    for (key = a[i]; i * 2 + 1 < len; i = child) 
    {
        child = 2 * i + 1;
        if (child + 1 < len && a[child] < a[child + 1]) ++child;
        if (key < a[child]) a[i] = a[child]; // a[i] is the parent
        else                break;
        a[child] = key;
    }
}

void heap_sort(int *a, int len)
{
    for (int i = len / 2 - 1; i >= 0; --i)
        heap_down(a, i, len);

    for (int i = len - 1; i > 0; --i)
    {
        swap(&a[0], &a[i]);
        heap_down(a, 0, i);
    }
}

int main()
{
    int a[10] = { 5, 9, 1, 4, 3, 5, 9, 0, 0, 1 };
    int len = 10;
    heap_sort(a, len);
    printf("heap sort : ");
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
