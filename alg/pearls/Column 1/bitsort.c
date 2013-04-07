/* Copyright (C) 1999 Lucent Technologies */
/* From 'Programming Pearls' by Jon Bentley */

/* bitsort.c -- bitmap sort from Column 1
 *   Sort distinct integers in the range [0..N-1]
 */

#include <stdio.h>

#define BITSPERWORD 32
#define SHIFT 5
#define MASK 0x1F
#define N 10000000
int a[1 + N/BITSPERWORD]; // 使用整型数组模拟定义1000万个位数组 

/*
 *  i>>SHIFT 是定位到数组中的哪个数中。34定位到第二个数中。
 *  i & MASK 是定位到这个数中的哪一位，从0开始，全部数字都要左移一位 
 *  set置1，clr置0，test测试
 */

void set(int i) {        a[i>>SHIFT] |=  (1<<(i & MASK)); }
void clr(int i) {        a[i>>SHIFT] &= ~(1<<(i & MASK)); }
int  test(int i){ return a[i>>SHIFT] &   (1<<(i & MASK)); }

int main()
{	int i;
	for (i = 0; i < N; i++)
		clr(i);
/*	Replace above 2 lines with below 3 for word-parallel init
	int top = 1 + N/BITSPERWORD;
	for (i = 0; i < top; i++)
		a[i] = 0;
 */
	while (scanf("%d", &i) != EOF)
		set(i);
	for (i = 0; i < N; i++)
		if (test(i))
			printf("%d\n", i);
	return 0;
}
