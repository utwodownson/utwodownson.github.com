#include "include/sort.h" 

int main()
{
    int len = 0;
    int data[MAX] = {0};
    char file[10] = "test.txt";

    srand((unsigned int)time(NULL));

    load_file(file, &len, data);
    shuffle(data, len);
    show_data(data, len);

#if SORT == INSERTION
    printf("insertion sort\n");
    insertion_sort(data, len);
#elif SORT == MERGE
    printf("merge sort\n");
    mergesort(data, len);
#elif SORT == URMERGE
    printf("merge sort unrecursion\n");
    mergesort_unr(data, len);
#elif SORT == BUBBLE
    printf("bubble sort\n");
    bubble_sort(data, len);
#endif

    show_data(data, len);
}

