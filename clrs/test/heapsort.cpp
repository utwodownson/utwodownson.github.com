#include <stdio.h>

void swap(int *a, int *b)
{
    *a = *a ^ *b;
    *b = *b ^ *a;
    *a = *b ^ *a;
}

void heapdown(int *a, int i, int len)
{
    int child, temp;
    for (temp = a[i]; 2 * i + 1 < len; i = child) {
        child = 2 * i + 1;
        if (child < len - 1 && a[child + 1] > a[child]) // len - 1 
            ++child;
        if (temp < a[child])
            a[i] = a[child];
        else
            break;
        a[child] = temp;
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
    int n = 0;
    int a[100];
    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);
    heapsort(a, n);
    for (int i = 0; i < n; ++i)
        printf("%d ", a[i]);
}
