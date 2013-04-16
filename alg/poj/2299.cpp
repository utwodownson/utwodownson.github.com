#include <stdio.h>

#define N 500001

int data[N];
int buf[N];
int n;
long long number;

void merge_sort(int left, int right)
{
    if (left >= right)
        return;
    int mid = (left & right) + ((left ^ right) >> 1);
    merge_sort(left, mid);
    merge_sort(mid + 1, right);

    int i = left;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= right) 
    {
        if (data[i] > data[j]) 
        {
            buf[k++] = data[j++];
            number += mid - i + 1;
        } 
        else 
            buf[k++] = data[i++];  
    }

    while (i <= mid)
        buf[k++] = data[i++];
    while (j <= right)
        buf[k++] = data[j++];

    for (int p = 0; p < k; ++p)
        data[left + p] = buf[p];
}

int main()
{
    while (scanf("%d", &n) != EOF) 
    {
        if (n == 0)
            break;
        for (int i = 0; i < n; ++i) 
            scanf("%d", &data[i]);
        number = 0;
        merge_sort(0, n - 1);
        printf("%lld\n", number);
    }
}
