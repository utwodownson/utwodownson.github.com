#include <stdio.h>

void insertion_sort(int *a, int len)
{
    int key, j;
    for (int i = 1; i < len; ++i)
    {
        key = a[i];
        j = i - 1;
        while (j >= 0 && a[j] >= key) 
            a[j + 1] = a[j--];
        a[j + 1] = key;
    }
}

int main()
{
    int a[10] = { 5, 10, 1, 1, 0, 4, 9, 3, 2, 1 };
    int len = 10;

    // the algorithm
    printf("Insertion Sort : ");
    insertion_sort(a, len);

    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
