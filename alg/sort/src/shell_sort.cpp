#include "../include/sort.h"

void shell_sort(int *data, int len)
{
    int key = 0;
    int j = 0;
    for (int step = len / 2; step > 0; step /= 2) {
        for (int i = step; i < len; ++i) {
            key = data[i];
            j = i - step;
            // for (j = i - step; j >= 0 && data[j] > key; j -= step)
            while (j >= 0 && data[j] > key) {
                data[j + step] = data[j];
                j -= step;
            }
            data[j + step] = key;
        }
    }
}
