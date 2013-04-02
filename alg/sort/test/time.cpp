#include "../include/sort.h" 
#include <time.h>

const int MAXN = 5000000;
int a[MAXN];
int b[MAXN], c[MAXN], d[MAXN];

int main()
{
    int i;
    srand(time(NULL));
    for (i = 0; i < MAXN; ++i)
        a[i] = rand() * rand(); //注rand()产生的数在0到65536之间

    for (i = 0; i < MAXN; ++i)
        d[i] = c[i] = b[i] = a[i];

    clock_t ibegin, iend;

    //快速排序
    printf("快速排序:  ");
    ibegin = clock();
    quick_sort(a, 0, MAXN - 1);
    iend = clock();
    printf("%d毫秒\n", (int) (iend - ibegin));


    //归并排序
    printf("归并排序:  ");
    ibegin = clock();
    mergesort(b, 0, MAXN - 1);
    iend = clock();
    printf("%d毫秒\n", (int) (iend - ibegin));

    //堆排序
    printf("堆排序:  ");
    ibegin = clock();
    heapsort(c, MAXN);
    iend = clock();
    printf("%d毫秒\n", (int) (iend - ibegin));

}
