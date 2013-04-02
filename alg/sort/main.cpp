#include "include/sort.h" 

int main()
{
    int len = 0;
    int data[MAX] = {0};
    char file[30] = "data/test.txt";

    srand((unsigned int) time(NULL));

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
#elif SORT == SELECT 
    printf("selection sort\n");
    selection_sort(data, len);
#elif SORT == SHELL 
    printf("shell sort\n");
    shell_sort(data, len);
#elif SORT == QUICK 
    printf("quick sort\n");
    quick_sort(data, 0, len - 1);
#elif SORT == HEAP 
    printf("heap sort\n");
    heapsort(data, len);
#endif

    show_data(data, len);
}

