#include <stdio.h>

//计算x^n 二分递归实现  by MoreWindows( http://blog.csdn.net/MoreWindows )
int power2(int x, unsigned int n)
{
    if (n == 0)
        return 1;
    else if (n == 1)
        return x;
    else {
        if (n % 2 == 1)
            return power2(x, n / 2) * power2(x, n / 2) * x;
        else
            return power2(x, n / 2) * power2(x, n / 2);
    }
}


/*
 *  23 = 10111(二进制)，所以只要将n化为二进制并由低位到高位依次判断如果第i位为1，则result *=x^(2^i)
 */

int power3(int x, unsigned int n)
{
    int result = 1;
    if (n == 0)
        return 1;
    else {
        while (n != 0) {
            if ((n & 1) != 0)
                result *= x;
            x *= x;
            n >>= 1;
        }
        return result;
    }
}

/*
 *  过滤掉低位的0  
 */

int power4(int x, unsigned int n)
{
    int result = 1;
    if (n == 0) 
        return 1;
    else {
        while ((n & 1) == 0) {
            n >>= 1;
            x *= x;     
        }
    }

    while (n != 0) {
        if ((n & 1) != 0) {
            result *= x;
        }
        x *= x; 
        n >>= 1;
    }
    return result;
}

int main()
{
    int num = 0;
    int m = 0;

    printf("the power's number is:\n");
    scanf("%d", &num);

    printf("the m is:\n");
    scanf("%d", &m);

    printf("the power is : %d\n", power2(num, m));
    printf("the power is : %d\n", power3(num, m));
    printf("the power is : %d\n", power4(num, m));
}
