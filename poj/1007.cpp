#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int N = 51;
const int M = 101;

int temp[N];
int number[N];

typedef struct _node {
    char line[N];
    int sum;
}Node;

Node node[M];

int n, m;
int total = 0;

int cmp(const void *a, const void *b)
{
    Node *left = (Node *)a;
    Node *right = (Node *)b;
    return (left->sum - right->sum);
}

void merge(int *array, int left, int mid, int right)
{
    int i = left;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= right) 
    {
        if (array[i] <= array[j])
        {
            temp[k++] = array[i++];
        }
        else 
        {
            temp[k++] = array[j++];
            total = total + mid - i + 1;
        }
    }

    while (i <= mid)
        temp[k++] = array[i++];
    while (j <= mid)
        temp[k++] = array[j++];

    for (int p = 0; p < k; ++p)
        array[left + p] = temp[p];
}

void merge_sort(int *array, int left, int right)
{
    if (left >= right)
        return;

    int mid = (left & right) + ((left ^ right) >> 1);
    merge_sort(array, left, mid);
    merge_sort(array, mid + 1, right);
    merge(array, left, mid, right);
}

void char_to_int(char *soure, int *det, int len)
{
    for (int i = 0; i < len; ++i)
        det[i] = (int)(soure[i] - 'A');
}

void show_node(int m)
{
    for (int i = 0; i < m; ++i) 
        printf("%s\n", node[i].line);
}

int main()
{
    while (scanf("%d%d", &n, &m) != EOF) 
    {
        for (int i = 0; i < m; ++i) 
        {
            scanf("%s", node[i].line);
            total = 0;
            memset(number, 0, n);
            char_to_int(node[i].line, number, n);
            merge_sort(number, 0, n - 1);
            node[i].sum = total;
        }
        qsort(node, m, sizeof(node[0]), cmp);
        show_node(m);
    }
}
