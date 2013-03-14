#include "include/sort.h"

void swap(int *left, int *right)
{
    *left = *left ^ *right;
    *right = *right ^ *left;
    *left = *left ^ *right;
}

void shuffle(int *array, int capacity)
{
    int temp = 0;
    for (int i = capacity - 1; i > 0; --i) {
        temp = rand() % (i + 1);
        if (temp != i)
            swap(&array[temp], &array[i]);
    }
}

















