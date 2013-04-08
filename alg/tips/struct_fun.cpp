#include <stdio.h>

struct square
{
    int length;
    int width;
    int height;
    int (*add)(int a, int b);
};

int square_add(int a, int b)
{
    return a + b;
}

int bulk(int length, int width, int height)
{
    struct square squ =
    {
        squ.length = length,
        squ.width = width,
        squ.height = height,
        add:square_add,
    };

    printf("Add() is %d\n", squ.add(100, 200));
    printf("Length is %d\n", squ.length);
    printf("Width is %d\n", squ.width);
    printf("Height is %d\n", squ.height);
    return squ.length * squ.width * squ.height;
}

int main()
{
    printf("The square is %d\n", bulk(100, 200, 300));
}
