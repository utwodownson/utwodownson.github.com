#include <stdio.h>

const int N = 1001;

int data[N];
int temp[N];

int n, m;
int total;

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
        if (data[i] <= data[j])
            temp[k++] = data[i++];
        else 
        {
            temp[k++] = data[j++];
            total += mid - i + 1;
        }
    }

    while (i <= mid)
        temp[k++] = data[i++];
    while (j <= right)
        temp[k++] = data[j++];

    for (int p = 0; p < k; ++p)
        data[left + p] = temp[p];
}

int main()
{
    scanf("%d", &n);
    int count = 0;
    while (n--) 
    {
        scanf("%d", &m);
        for (int j = 0; j < m; ++j)
            scanf("%d", &data[j]);

        total = 0;
        merge_sort(0, m - 1);
        printf("Scenario #%d:\n", ++count);
        printf("%d\n", total);
        printf("\n");
    }
}
