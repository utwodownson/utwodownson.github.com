#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int a[101][501];
    int b[101];
    int n, d, flag;
    while (scanf("%d%d", &n, &d) != EOF && (n + d)) {
        flag = 0;
        memset(b, 1, sizeof(int) * 101);

        for (int i = 0; i < d; ++i) {
            for (int j = 0; j < n; ++j) { 
                scanf("%d", &a[i][j]);
                b[j] = b[j] && a[i][j];
            }
        }
        for (int i = 0; i < n; ++i) {
            if (b[i]) {
                flag = 1;
                break;
            }
        }
        if (flag)
            printf("yes\n");
        else
            printf("no\n");
    }
}
