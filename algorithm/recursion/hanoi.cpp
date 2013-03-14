#include <stdio.h>

void move(char x, char y)
{
    printf("%c-->%c\n", x, y);
}

void hanoi(int n, char left, char mid, char right)
{
    if (n == 1)
        move(left, right);
    else {
        hanoi(n - 1, left, right, mid);
        move(left, right);
        hanoi(n - 1, mid, left, right);
    }
}

int main()
{
    int number = 0;
    printf("input the number of diskes\n");
    scanf("%d", &number);

    printf("The step to move %d diskes:\n", number);
    hanoi(number, 'A', 'B', 'C');
}
