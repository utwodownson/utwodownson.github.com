##2013-04-15
    尾递归，最优二叉查找树，兼容子集，快速排序优化（三路取中，尾递归）
QUICKSORT (A, p, r )
    while p < r
    do Partition and sort the small subarray ?rst
q ← PARTITION(A, p, r )
    if q ? p < r ? q
then QUICKSORT (A, p, q ? 1)
    p ← q + 1
else QUICKSORT (A, q + 1, r )
    r ← q ? 1

while(p<r)//直到把右半边数组划分成最多只有一个元素为止，就排完了！不能用if哦，用if的话，右半边数组就只被划分一次。
{
    q=RandomPartition(a,p,r);
    QuickSort(a,p,q-1);
    p=q+1;
}

01背包问题 哈弗曼编码 贪心算法16.1的迭代尾递归 看完贪心 看完01背包
