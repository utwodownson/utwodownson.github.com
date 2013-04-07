// 快速排序的非递归算法
//
#include <stdio.h>

#define MAX 16       //元素个数的最大值
#define NIL -1      //失败标志

typedef struct _queue{ 
    int d[MAX];     //用数组作为队列的储存空间
    int front, rear; //指示队头位置和队尾位置的变量
}SEQUEUE;           //顺序队列类型定义

SEQUEUE sq ;         //定义顺序队列

int R[MAX + 1] = { 0, 9, 2, 13, 5, 10, 16, 17, 11, 4, 6, 8, 1, 7 }; //用数组来储存待排序元素,R[0]为辅助单元

int ENQUEUE(int x)                              //顺序循环队列的入队算法
{ 
    if (sq.front == (sq.rear + 1) % MAX) //满队，入队失败
        return NIL;
    else { 
        sq.rear = (sq.rear + 1) % MAX;  //调整队尾变量
        sq.d[sq.rear] = x;                 //数据入队
        return 1 ; 
    }                       //入队成功
}

int DEQUEUE()                      //循环队列的出队算法
{ 
    if (sq.front == sq.rear)           //空队，出队失败
        return NIL;
    else { 
        sq.front = (sq.front + 1) % MAX ; //调整队首变量
        return (sq.d[sq.front]);
    }           //返回队首元素
}

void QUICKSORT()                           //快速排序算法
{ 
    int i, j, l, r;
    while (1) {
        l = DEQUEUE();                       //排序区间起点序号出队
        r = DEQUEUE();                       //排序区间终点序号出队
        if (l == NIL) return;                 //没有待排序区间,算法结束。
        i = l; j = r; R[0] = R[i];         //左右指针和基准值初始化

        while (i < j) {                    //只要存在无序区间
            while (R[j] >= R[0] && i < j) j--; //扩充大值区间
            if (i<j) R[i++] = R[j];              //将小值记录移入小值区间
            while (R[i] <= R[0] && i < j) i++; //扩充小值区间
            if (i<j) R[j--] = R[i];              //将大值记录移入大值区间
        }
        R[i] = R[0];                          //将基准记录放入最终位置

        if (l < i - 1) {                          //如果小值区间存在
            ENQUEUE (l);                         //小值区间起点序号入队
            ENQUEUE (i - 1);                       //小值区间终点序号入队
        }
        if (i + 1 < r) {                          //如果大值区间存在
            ENQUEUE (i + 1);                       //大值区间起点序号入队
            ENQUEUE (r);                         //大值区间终点序号入队
        }
    }
}
int main ()
{  
    sq.front = sq.rear = MAX - 1;           //初始化空队
    ENQUEUE(1);                          //排序区间起点序号入队
    ENQUEUE(13);                         //排序区间终点序号入队
    QUICKSORT();                        //调用快速排序算法
    while (1) ;                           //在这一行设置断点,中止程序运行,以                                       //便观察程序运行的结果
}
