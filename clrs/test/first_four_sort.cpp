#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void swap(int *a, int *b)
{
    *a = *a ^ *b;
    *b = *b ^ *a;
    *a = *b ^ *a;
}

void show(int *a, int len)
{
    for (int i = 0; i < len; ++i)
        printf("%d ", a[i]);
    printf("\n");
}

void merge(int *a, int left, int mid, int right)
{
    int len = right - left + 1;
    int *buff = (int *) malloc(sizeof(int) * len);
    if (buff == NULL)
        return;
    int i = left;
    int j = mid + 1;
    int k = 0;
    while (i <= mid && j <= right) 
        buff[k++] = a[i] < a[j] ? a[i++] : a[j++];

    while (i <= mid)
        buff[k++] = a[i++];

    while (j <= right)
        buff[k++] = a[j++];

    for (int p = 0; p < k; ++p)
        a[left + p] = buff[p];

    if (buff != NULL)
        free(buff);
}

void insertion(int *a, int len)
{
    int key, j;
    for (int i = 1; i < len; ++i) {
        key = a[i];
        j = i - 1;
        while (j >= 0 && key < a[j]) a[j + 1] = a[j--];
        a[j + 1] = key;
    }
}

void merger(int *a, int left, int right)
{
    int mid;
    if (left < right) {
        mid = left + (right - left) / 2;
        merger(a, left, mid);
        merger(a, mid + 1, right);
        merge(a, left, mid, right);
    }
}

void mergeur(int *a, int len)
{
    int left_min, left_max, right_min, right_max, next;
    int *buff = (int *) malloc(sizeof(int) * len);
    if (buff == NULL)
        return;
    
    for (int i = 1; i < len; i *= 2) {
        for (left_min = 0; left_min < len - i; left_min = right_max) {
            left_max = right_min = left_min + i;
            right_max = right_min + i;
            if (right_max > len)
                right_max = len;
            next = 0;
            while (left_min < left_max && right_min < right_max)
                buff[next++] = a[left_min] < a[right_min] ? a[left_min++] : a[right_min++];
            while (left_min < left_max)
                a[--right_min] = a[--left_max];
            printf("\t");
            show(a, 10);
            while (next > 0)
                a[--right_min] = buff[--next];
            printf("\t");
            show(a, 10);
        }
    }

    if (buff != NULL)
        free(buff);
}

void heapdown(int *a, int i, int len)
{
    int child, temp;
    for (temp = a[i]; 2 * i + 1 < len; i = child) {
        child = 2 * i  + 1;
        while (child + 1 < len && a[child + 1] > a[child]) 
            ++child;
        if (temp < a[child])
            a[i] = a[child];
        else
            break;
        a[child] = temp;
    }
}

void heap(int *a, int len)
{
    for (int i = len / 2 - 1; i >= 0; --i)
        heapdown(a, i, len);
    
    for (int i = len - 1; i > 0; --i) {
        swap(&a[0], &a[i]);
        heapdown(a, 0, i);
    }
}

int position(int *a, int low, int high)
{
    int key = a[low];
    int i = low;
    int j = high;
    while (i < j) {
        while (i < j && a[j] >= key) --j;
        if (i < j) a[i++] = a[j];
        printf("\t");
        show(a, 10);
        while (i < j && a[i] <= key) ++i;
        if (i < j) a[j--] = a[i];
        printf("\t");
        show(a, 10);
    }
    a[i] = key;
    return (i);
}

void quick_sort(int *a, int low, int high)
{
    int flag;
    if (low < high) {
        flag = position(a, low, high);
        quick_sort(a, low, flag - 1);
        quick_sort(a, flag + 1, high);
    }
}

int main()
{
    int a[10] = { 9, 3, 2, 5, 3, 6, 10, 7, 1, 0 };
    int b[10] = { 9, 3, 2, 5, 3, 6, 10, 7, 1, 0 };
    int c[10] = { 9, 3, 2, 5, 3, 6, 10, 7, 1, 0 };
    int d[10] = { 9, 3, 2, 5, 3, 6, 10, 7, 1, 0 };
    int e[10] = { 9, 3, 2, 5, 3, 6, 10, 7, 1, 0 };
    int len = 10;

    printf("the array is\t:\t\n");
    show(a, 10);

    insertion(a, len);
    printf("insertion_sort\t:\t");
    show(a, len);

    merger(b, 0, len - 1);
    printf("merge recursion\t:\t");
    show(b, len);

    mergeur(c, len);
    printf("merge unr\t:\t");
    show(c, len);

    heap(d, len);
    printf("heapsort\t:\t");
    show(d, len);

    quick_sort(e, 0, 9);
    printf("quick sort\t:\t");
    show(e, len);
}
