#include <stdio.h>

void insertion_sort(int *a, int len)
{
    int key, j;
    for (int i = 1; i < len; ++i)
    {
        j = i - 1;
        key = a[i];
        while (j >=0 && key <= a[j]) 
            a[j + 1] = a[j--];
        a[j + 1] = key;
    }
}

int main()
{
    int a[10] = { 5, 1, 3, 2, 4, 7, 9, 0, 2, 10 };
    int len = 10;
    insertion_sort(a, len);
    printf("insertion sort : ");
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}
