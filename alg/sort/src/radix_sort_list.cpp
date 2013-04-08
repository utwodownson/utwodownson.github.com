#include <iostream>
using namespace std;

const int base = 10;

struct wx
{
    int num;
    wx *next;
    wx()
    {
        next = NULL;
    }
};

wx *headn, *curn, *box[base], *curbox[base];

void basesort(int t)
{
    int i, k = 1, r, bn;
    for (i = 1; i <= t; ++i) 
        k *= base;

    r = k * base;

    for (i = 0; i < base; ++i)
        curbox[i] = box[i];

    for (curn = headn->next; curn != NULL; curn = curn->next) {
        bn = (curn->num % r) / k;
        curbox[bn]->next = curn;
        curbox[bn] = curbox[bn]->next;
    }
    curn = headn;
    for (i = 0; i < base; ++i) {
        if (curbox[i] != box[i]) {
            curn->next = box[i]->next;
            curn = curbox[i];
        }
    }
    curn->next = NULL;
}

void printwx()
{
    for (curn = headn->next; curn != NULL; curn = curn->next)
        cout << curn->num << ' ';
    cout<<endl;
}

int main()
{
    int i, n, bit_num = 0, maxn = 0;
    curn = headn = new wx;
    cin >> n;
    for (i = 0; i < base; ++i)
        curbox[i] = box[i] = new wx;

    for (i = 1; i <= n; ++i) {
        curn = curn->next = new wx;
        cin >> curn->num;
        maxn = max(maxn, curn->num);
    }

    while (maxn / base > 0) {
        maxn /= base;
        bit_num++;
    }

    for (i = 0; i <= bit_num; ++i)
        basesort(i);

    printwx();
}
