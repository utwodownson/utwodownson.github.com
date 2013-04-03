#include <stdio.h>

int sum(int r, int m, int y)
{
    double result = 0.0;
    result = (double) m;
    for (int i = 0; i < y; ++i) 
        result = (100 + r) * result / 100;
    return ((int) result);
}

int main()
{
    int r, m, y, result;
    while (scanf("%d%d%d", &r, &m, &y) != EOF) {
        result = 0;
        result = sum(r, m, y);
        printf("%d\n", result);
    }
}
