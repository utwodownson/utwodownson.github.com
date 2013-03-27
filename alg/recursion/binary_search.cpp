#include <stdio.h>

int binary_search(int *data, int low, int high, int key)
{
    int temp = 0;
    if (low <= high) // low <= high
    {
        temp = (low & high) + ((low ^ high) >> 1);
        if (data[temp] == key) 
            return 1;
        else if (data[temp] < key)
            binary_search(data, temp + 1, high, key);
        else 
            binary_search(data, low, temp - 1, key);
    }
    return -1; // have not return -1
}

int main()
{
    int num = 0;
    int data[10];
    int key;
    printf("Enter some numbers:\n");
    scanf("%d", &num);
    for (int i = 0; i < num; ++i)
        scanf("%d", &data[i]);

    printf("you will fine:\n");
    scanf("%d", &key);
    if (binary_search(data, 0, num - 1, key))
        printf("find %d\n", key);
    else
        printf("not find %d\n", key);
}
