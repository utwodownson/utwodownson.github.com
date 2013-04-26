---
layout: page
title: About 
---

{% highlight javascript %}
#include <stdio.h>
#include <string.h>

void swap(char *a, char *b)
{
    *a = *a ^ *b;
    *b = *a ^ *b;
    *a = *a ^ *b;
}

void reverse(char *str, int i, int j)
{
    for (; i < j; ++i, --j) 
        swap(&str[i], &str[j]);
}

void rightrotate(char *str, int k, int n)
{
    k = k % n;
    reverse(str, 0, n - k - 1);
    reverse(str, n - k, n - 1);
    reverse(str, 0, n - 1);
}

void leftrotate(char *str, int k, int n)
{
    k = k % n;
    reverse(str, 0, k - 1);
    reverse(str, k, n - 1);
    reverse(str, 0, n - 1);
}

int main()
{
    //char *a = "the is woring"; // the is woring is a const string, only *a can change
    char b[100] = "1234 abc";
    char c[100] = "1234 abc";
    int len_b = strlen(b);
    rightrotate(b, 2, len_b);
    printf("b right rotate: %s\n", b);

    leftrotate(c, 2, len_b);
    printf("c left rotate: %s\n", c);
}
{% endhighlight %}
