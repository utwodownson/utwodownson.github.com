#include <stdio.h>
#include <math.h>

int main()
{
    int n, p, t, result, temp;
    while (scanf("%d%d", &n, &p) != EOF) {
        result = 0;
        for (int i = 0; i < n; ++i) {
            scanf("%d", &t);
            temp = (int) pow(t, p);
            result += (temp > 0 ? temp : 0);
        }
        printf("%d\n", result);
    }
}
