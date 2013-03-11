#include "include/sort.h" 

void insertion_sort(int *data, int len)
{
    int now = 0;
    int key = 0;
    for (int i = 1; i < len; ++i)
    {
        key = data[i];
        now = i;
        while (now > 0 && data[now - 1] > key)
        {
            data[now] = data[now - 1];
            --now;
        }
        data[now] = key;
    }
}

#ifdef TESTINSERTION
int main()
{
    int number = 0;
    scanf("%d", &number);
    int data[100] = {0};
    for (int i = 0; i < number; ++i)
    {
        scanf("%d", &data[i]);
    }
    insertion_sort(data, number);
    for (int i = 0; i < number; ++i)
    {
        printf("%d ", data[i]);
    }
    printf("\n");
}
#endif

