#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct _node {
    int x;
    int y;
}Point;

Point a[2501]; // 把每一层变成一个图

int main()
{
    int move[60]; // 记录每一层的位置
    int h, l, n, num, flag, result, dis;
    while (scanf("%d", &n) != EOF) {
        for (int i = 0; i < n; ++i) {
            flag = 0;
            result = 0;
            dis = 0;
            scanf("%d%d", &h, &l);
            for (int j = 0; j < h; ++j) {
                for (int q = 0; q < l; ++q) {
                    scanf("%d", &num);
                    if (num != -1) {
                        a[num].x = j;
                        a[num].y = q;
                        ++flag;
                    }
                }
            }
            memset(move, 0, sizeof(int) * 60);
            for (int j = 1; j <= flag; ++j) { // j form 1 not 0
                result += a[j].x * 20; 
                dis = MIN(abs(a[j].y - move[a[j].x]), l - abs(a[j].y - move[a[j].x]));
                result += dis * 5;
                move[a[j].x] = a[j].y;
            }
            printf("%d\n", result);
        }
    }
}


