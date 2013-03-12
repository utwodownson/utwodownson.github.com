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
    insertion_sort(data, len);
#elif SORT == MERGE
    mergesort(data, len);
#endif

    show_data(data, len);
}

