#include "include/sort.h"

void merge(int *data, int left, int mid, int right)
{
    int length = right - left + 1;
    int *buffer = NULL;
    int i = left;
    int j = mid + 1;
    int k = 0;

    buffer = (int *)malloc(sizeof(int) * length);   // fit the recursion merge, if unrecursion function this line can mv to the merge_sort function
    if (buffer == NULL) {
        fputs("Error : out of memory\n", stderr);     
        abort();
    }
    
    while (i <= mid && j <= right) 
        buffer[k++] = (data[i] <= data[j]) ? data[i++] : data[j++]; // better
    while (i <= mid)
        buffer[k++] = data[i++];
    while (j <= right)
        buffer[k++] = data[j++];

    for (int p = 0; p < k; ++p)
        data[left + p] = buffer[p];

    if (buffer != NULL)
        free(buffer); // donnot forget free
}

// recursion
void mergesort(int *data, int left, int right)
{
    int mid = 0;
    if (left < right) {  // notice
        // mid = (left + right) / 2 --> if the left and right are too big 
        // mid = left / 2 + right / 2 --> wrong 
        mid = (left & right) + ((left ^ right) >> 1); // right
        mergesort(data, left, mid);
        mergesort(data, mid + 1, right);
        merge(data, left, mid, right);
    }
}

// unrecursion
// 将原数组划分为left[min...max] 和 right[min...max]两部分
void mergesort_unr(int *list, int length)
{
    int left_min, left_max, right_min, right_max, next;
    int *tmp = (int*)malloc(sizeof(int) * length);

    if (tmp == NULL) {
        fputs("Error: out of memory\n", stderr);
        abort();
    }

    for (int i = 1; i < length; i *= 2) {   // i为步长，1,2,4,8……
        for (left_min = 0; left_min < length - i; left_min = right_max) {
            /*
             * left_max其实是左半部的长度，right_max其实是右半部的长度，
             * 所以应该叫left_len、righ_len更合适一些。
             */
            right_min = left_max = left_min + i; //right_min取left_max的值，以下要用到left_max的值才不会改变left_max原先值
            right_max = left_max + i;   // right_max标记下一次的开始位置

            if (right_max > length) //如果right_max超出了数组长度，则right_max等于数组长度
                right_max = length;

            next = 0;
            while (left_min < left_max && right_min < right_max) //tmp[next]存储子数组中的最小值
                tmp[next++] = list[left_min] > list[right_min] ? list[right_min++] : list[left_min++];

            /*
             * 左半部和右半部从长度为1开始处理，每次处理后，左半部和右半部
             * 组成的段是排好序的，所以在下次处理中，每个段的左半部和右半部
             * 已经是排好序的。如果判断条件为真，说明右半部中当前元素已经全
             * 部放在tmp中，而左半部中剩余的元素肯定比已经放在tmp中的元素大，
             * 因此循环将左半部中剩余的元素放在段的后面。
             */
            while (left_min < left_max)  //如果left_min小于left_max，则将最小值原封不动地赋给list[--right_min]，right_min 要先减1
                list[--right_min] = list[--left_max];

            while (next > 0) //将tmp[next]存储的最小值放入list[--right_min]中
                list[--right_min] = tmp[--next];
        }
    }
    free(tmp);
}
