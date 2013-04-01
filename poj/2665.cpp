#include <stdio.h>

int main()
{
    int l, m, start, end;
    int result;
    while (scanf("%d%d", &l, &m) != EOF && (l + m)) {
        result = l + 1;
        for (int i = 0; i < m; ++i) {
            scanf("%d%d", &start, &end);
            result = result - end + start - 1; 
        }
        printf("%d\n", result);
    }
}
