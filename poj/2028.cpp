#include <stdio.h>
#include <string.h>

int table[101];

int main()
{
    int n, q, m, day, max;

    while (scanf("%d%d", &n, &q) != EOF)
    {
        if (n == 0 && q == 0) break;
        memset(table, 0, sizeof(int) * 101);
        max = q - 1;
        for (int i = 0; i < n; ++i)
        {
            scanf("%d", &m);
            for (int j = 0; j < m; ++j)
            {
                scanf("%d", &day);
                table[day]++;
            }
        }
        day = 0;
        for (int i = 0; i < 101; ++i)
        {
            if (table[i] > max)
            {
                max = table[i];
                day = i;
            }
        }
        printf("%d\n", day);
    }
}
