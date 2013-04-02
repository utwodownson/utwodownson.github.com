#include <stdio.h>
#include <string.h>
#include <math.h>

int value(char a, char *set)
{
    int len = strlen(set);
    for (int i = 0; i < len; ++i) {
        if (a == set[i])
            return i;
    }
    return -1;
}

int RK(char *t, char *m, char *set)
{
    int len = strlen(set);
    int tlen = strlen(t);
    int mlen = strlen(m);
    int h = (int)pow(len, mlen - 1);

    int up = 0;
    int down = 0;

    for (int i = 0; i < mlen; ++i)
    {
        up = len * up + value(t[i], set);
        down = len * down + value(m[i], set);
    }

    for (int i = 0; i < tlen - mlen; ++i)
    {
        if (up == down)
            return i;
        else 
            up = (up - h * value(t[i], set)) * len + value(t[i + mlen], set);
    }
    return -1;
} 

int main()
{
    char set[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    char t[] = "258569236589780";
    char m[] = "2365";
    int i = RK(t, m, set);
    printf("%d\n", i);
}
