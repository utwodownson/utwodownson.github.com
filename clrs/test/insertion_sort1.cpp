#include <stdio.h>

void insertion_sort(int *a, int len)
{
    int j = 0;
    int key = 0;

    for (int i = 1; i < len; ++i) {
        key = a[i];    
        j = i - 1;
        while (j >= 0 && a[j] > key) a[j + 1] = a[j--]; 
        a[j + 1] = key; 
    }
}

int main()
{
    int a[100];
    int n = 0;
    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);
    insertion_sort(a, n);
    for (int i = 0; i < n; ++i)
        printf("%d ", a[i]);
}
