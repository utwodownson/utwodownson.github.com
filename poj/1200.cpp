#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX 16000000

int table[MAX];
int c[128];
char text[1000000];

// get_value 128 ascll 
void get_value(int nc, int len)
{
    int num = 0;
    for (int i = 0; i < len; ++i)
    {
        if (c[text[i]] == 0)
            c[text[i]] = ++num;
        // the better
        if (num == nc)
            break;
    }
}

int main()
{
    int count = 0;
    int sum = 0;
    memset(c, 0, 128);
    memset(table, 0, MAX);

    int n, nc;
    scanf("%d%d", &n, &nc);

    int h = (int) pow(nc, n - 1);

    scanf("%s", text);
    int len = strlen(text);
    get_value(nc, len);

    for (int i = 0; i < n; ++i)
        sum = sum * nc + c[text[i]]; 
    ++table[sum];
    ++count;

    for (int i = 0; i < len - n; ++i) // len - n 
    {
        sum = (sum - c[text[i]] * h) * nc + c[text[i + n]];
        if (table[sum] == 0)
            ++count;
        ++table[sum];
    }
    printf("%d\n", count);
}





















