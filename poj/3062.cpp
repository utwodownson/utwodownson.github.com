#include <stdio.h>
#include <string.h>

int main()
{
    char string[110];
    while (gets(string) != NULL)
        puts(string);
}
