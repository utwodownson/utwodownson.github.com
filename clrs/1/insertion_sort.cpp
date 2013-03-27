#include <stdio.h>
// insertion sort one 5min
// two times
void insertion_sort(int *a, int len)
{
    int key = 0, j = 0;
    for (int i = 1; i < len; ++i) {
        key = a[i];
        j = i - 1;
        while (j >= 0 && a[j] > key) 
            a[j + 1] = a[j--];
        a[j + 1] = key; // j + 1
    }
}

int main()
{
    int a[10];
    int n = 0;
    scanf("%d", &n);
    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }
    insertion_sort(a, n);
    for (int i = 0; i < n; ++i) {
        printf("%d ", a[i]);
    }
    printf("%d\n");
}
