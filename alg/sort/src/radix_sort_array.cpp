#include <iostream>
#include <cstring>
using namespace std;

int arr[100], res[100], hash[10];
int n;

int maxbit()
{
    int _max = 0;
    int temp[100];
    for (int i = 0; i < n; ++i)
        temp[i] = arr[i];

    for (int i = 0; i < n; ++i) {
        int tt = 1;
        while (temp[i] / 10 > 0) {
            ++tt;
            temp[i] /= 10;
        }
        if (_max < tt)
            _max = tt;
    }
    cout << "_max : " << _max << endl;
    return _max;
}

void radixSort()
{
    memset(res, 0, sizeof(res));
    int nbit = maxbit(); // 位数
    int tmp;
    int radix = 1;

    // 以下和计数排序一样
    for(int i = 1; i <= nbit; ++i)
    {
        for(int j = 0; j < 10; ++j)
            hash[j] = 0;

        for(int j = 0; j < n; ++j) {
            tmp = (arr[j] / radix) % 10;
            ++hash[tmp];
        }

        for(int j = 1; j < 10; ++j)
            hash[j] += hash[j - 1];

        for(int j = n - 1; j >= 0; --j) {
            tmp = (arr[j] / radix) % 10;
            --hash[tmp];
            res[hash[tmp]] = arr[j];
        }

        for(int j = 0; j < n; ++j)
            arr[j] = res[j];
        radix *= 10;
    }
}

int main()
{
    freopen("input.txt", "r", stdin);
    cout << "输入元素个数: ";
    cin >> n;
    cout << "输入" << n << "个元素: " << endl;

    for (int i = 0; i < n; ++i)
        cin >> arr[i];

    radixSort();

    cout << "排序后结果为: " << endl;
    for (int i = 0; i < n; ++i)
        cout << res[i] << " ";
    cout << endl;
}
