#include <stdio.h>
int main()
{
    double a[10000], b[10000];
    int i, n;
    while (scanf("%d", &n) != EOF)
    {
        for (i = 0; i < n; ++i)
            scanf("%lf%lf", &a[i], &b[i]);
        a[i] = a[0];
        b[i] = b[0];
        printf("%d ", n);
        for (i = 0; i < n; ++i)
            printf("%0.6f %0.6f ", (a[i] + a[i + 1]) / 2, (b[i] + b[i + 1]) / 2);
        printf("\n");                
    }
}
/*
 * Time Limit Exceeded
 *
typedef struct _node{
    double x;
    double y;
}Node;
Node a[30000];
int main()
{
    int n = 0;
    double mx;
    double my;
    while (scanf("%d", &n) != EOF)
    {
        for (int i = 0; i < n; ++i)
            scanf("%lf%lf", &a[i].x, &a[i].y);
        a[n].x = a[0].x;
        a[n].y = a[0].y;
        printf("%d", n);
        for (int i = 0; i < n; ++i)
        {
            mx = (a[i].x + a[i + 1].x) / 2.0;
            my = (a[i].y + a[i + 1].y) / 2.0;
            printf(" %.6lf %.6lf", mx, my);
        }
        printf("\n");
    }
}
*/
