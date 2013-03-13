##排序


wiki上的排序分类：

* 交换排序法    | [冒泡排序](http://zh.wikipedia.org/wiki/%E5%86%92%E6%B3%A1%E6%8E%92%E5%BA%8F) | 鸡尾酒排序 | 奇偶排序 | 梳排序 | 侏儒排序 | [快速排序](http://zh.wikipedia.org/wiki/%E5%BF%AB%E9%80%9F%E6%8E%92%E5%BA%8F) | 臭皮匠排序 | Bogo排序

* 选择排序法    | [选择排序](http://zh.wikipedia.org/wiki/%E9%80%89%E6%8B%A9%E6%8E%92%E5%BA%8F) | [堆排序](http://zh.wikipedia.org/wiki/%E5%A0%86%E6%8E%92%E5%BA%8F) | Smooth排序 | 笛卡尔树排序 | 锦标赛排序 | 循环排序

* 插入排序法    | [插入排序](http://zh.wikipedia.org/wiki/%E6%8F%92%E5%85%A5%E6%8E%92%E5%BA%8F) | [希尔排序](http://zh.wikipedia.org/wiki/%E5%B8%8C%E5%B0%94%E6%8E%92%E5%BA%8F) | 二叉查找树排序 | 图书馆排序 | Patience排序

* 归并排序法    | [归并排序](http://zh.wikipedia.org/wiki/%E5%BD%92%E5%B9%B6%E6%8E%92%E5%BA%8F) | 多相归并排序 | Strand排序

* 分布排序法    | 美国旗帜排序 | 珠排序 | [桶排序](http://zh.wikipedia.org/wiki/%E6%A1%B6%E6%8E%92%E5%BA%8F) | 爆炸排序 | [计数排序](http://zh.wikipedia.org/wiki/%E8%AE%A1%E6%95%B0%E6%8E%92%E5%BA%8F) | 鸽巢排序 | 相邻图排序 | [基数排序](http://zh.wikipedia.org/wiki/%E5%9F%BA%E6%95%B0%E6%8E%92%E5%BA%8F) | 闪电排序 | 插值排序

* 混合排序法    | Tim排序 | 内省排序 | Spread排序 | 反移排序 | J排序

* 其他 | 双调排序器 | Batcher归并网络 | 两两排序网络

本文只介绍`《算法导论》`里边提到的排序算法，按照出现的顺序，代码实现如下：

##[Insertion Sort](insertion_sort.cpp)

#####从后向前，在已排序的数组中插入key的元素。
    * 发明者Hollerith创立的电脑制表记录公司（CTR）在1924年更名为IBM。
    * 通常采用in-place排序（即只需用到O(1)的额外空间的排序）。
    * 插入排序在工业级库中也有着广泛的应用，在STL的sort算法和stdlib的qsort算法中，都将插入排序作为快速排序的补充，用于少量元素的排序（通常为8个或以下）。

##[Merge Sort](mergesort.cpp)

####递归
    拆分成两个数组合并, 然后求mid，合并前一半，合并后一半。

####非递归
    从下至上2,4,8,...,length合并

    * 4个标志位
        * left_min 开始位置
        * right_min 右侧开始位置，同时是放回数组的标志位。每次整理从right_min-->0的数组
        * left_max 限制左边的标志位
        * right_max 下一次开始的位置 

##[Bubble Sort](bubble_sort.cpp)

#####通过两两比较交换，把最小的元素“浮”到数据的顶端。
    * 在最坏的情况，冒泡排序需要O(n^2)次交换，而插入排序只要最多O(n)交换。
    * 冒泡排序如果能在内部循环第一次执行时，使用一个旗标来表示有无需要交换的可能，也有可能把最好的复杂度降低到O(n);
    * 若在每次走访数列时，把走访顺序和比较大小反过来，也可以稍微地改进效率。有时候称为`往返排序`，因为算法会从数列的一端到另一端之间穿梭往返。
