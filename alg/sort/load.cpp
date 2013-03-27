#include "include/sort.h"

int load_file(char *filename, int *number, int *array) 
{
    if (filename == NULL)
        return ERROR;

    FILE *fp = NULL;
    int capacity = 0;

    fp = fopen(filename, "rb");

    if (fp == NULL) {
        printf("can not open the file\n");
        return ERROR;
    }

    fscanf(fp, "%d", &capacity);
    *number = capacity;
    
    for (int i = 0; i < capacity; ++i) 
        fscanf(fp, "%d", &array[i]);

    return PROCESSED;
}

void show_data(int *array, int capacity)
{
    for (int i = 0; i < capacity; ++i)
        printf("%d ", array[i]);

    printf("\n");
}
