#include <stdio.h>

#define MAX 1000

typedef struct _priorityqueue {
    int capacity;
    int n;
    int *a;
}PQ;

PQ *init(int max)
{
    PQ *list = NULL;
    list = (PQ *) malloc(sizeof(PQ));
    if (NULL != list) {
        list->a = (int *) malloc(sizeof(int) * max);
        if (list->a != NULL) {
            list->capacity = max;
            list->n = 0; 
        }
    }
    return list;
}

int empty(PQ *list)
{
    return (list->n == 0);
}

void heapup(PQ *list, int x)
{
    int i = -1;
    if (list->n == list->capacity) {
        printf("The Priority Queue is full\n");
        return;
    }
    
    for (i = list->n; i > 0 && x > list->a[(i-1)/2]; i = (i - 1) / 2) 
        list->a[i] = list->a[(i-1)/2];

    list->a[i] = x;
    list->n++;
}

void heapdown(PQ *list, int i)
{
    int len = list->n;
    int child, temp;
    if (empty(list))
        return;

    for (temp = list->a[i]; i * 2 + 1 < len; i = child) {
        child = i * 2 + 1;
        if (child < len - 1 && list->a[child + 1] > list->a[child])
            ++child;
        if (temp < list->a[child])
            list->a[i] = list->a[child];
        else
            break;
        list->a[child] = temp;
    }
}

