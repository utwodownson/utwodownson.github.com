#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void counting_sort(int *a, int len, int *c, int n)
{
    memset(c, 0, sizeof(int) * n);
    for (int i = 0; i < len; ++i)  ++c[a[i]];
    for (int i = 1; i < n; ++i) c[i] = c[i] + c[i - 1];

    int *b = (int *) malloc(sizeof(int) * len);
    if (b == NULL)
        return;

    for (int i = 0; i < len; ++i) {
        b[c[a[i]] - 1] = a[i]; 
        --c[a[i]]; // forget the line
    }

    for (int i = 0; i < len; ++i)
        a[i] = b[i];

    if (b != NULL)
        free(b);
}

int main()
{
    int a[10] = { 6, 1, 0, 2, 4, 2, 6, 8, 3, 2 };
    int len = 10;
    int n = 9;
    int *c = (int *) malloc(sizeof(int) * len);
    if (c == NULL)
        return -1;

    counting_sort(a, len, c, n);
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
    if (c != NULL)
        free(c);
}
