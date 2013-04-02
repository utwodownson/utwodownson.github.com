#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 110

int b[MAX];

void input(int n, int d)
{
    int temp;
    for (int i = 0; i < d; ++i)
        for (int j = 0; j < n; ++j) {
            scanf("%d", &temp);
            if (!temp)
                b[j] = 0;
        }
}

int find(int n)
{
    for (int i = 0; i < n; ++i) {
        if (b[i])
            return 1;
    }
    return 0;
}

int main()
{
    int n, d, flag;
    while (scanf("%d%d", &n, &d), n || d) {
        flag = 0;
        memset(b, 1, sizeof(b));

        input(n, d);
        flag = find(n);

        if (flag)
            printf("yes\n");
        else
            printf("no\n");
    }
}
