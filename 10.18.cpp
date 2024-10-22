#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
using namespace std;
int tmp[100004];
//void merge_sort(int a[], int left, int right)
//{
//    if (left >= right) return;
//    int mid = (left + right) / 2;
//    merge_sort(a, left, mid);
//    merge_sort(a, mid + 1, right);
//    int i = left, j = mid + 1;
//    int k = left;
//    while (i <= mid && j <= right)
//    {
//        if (a[i] < a[j])tmp[k++] = a[i++];
//        else tmp[k++] = a[j++];
//    }
//    while (i <= mid) tmp[k++] = a[i++];
//    while (j <= right) tmp[k++] = a[j++];
//    for (int i = left; i < k; ++i) a[i] = tmp[i];
//}
//int main()
//{
//    int n;
//    int a[100004];
//    scanf("%d", &n);
//    for (int i = 0; i < n; i++) scanf("%d", &a[i]);
//    merge_sort(a, 0, n - 1);
//    for (int i = 0; i < n; i++) printf("%d ", a[i]);
//    return 0;
//
//}
#include <iostream>
using namespace std;
int max_num(int num[], int n, int len)
{
    int maxnum = 0;
    for (int i = n; i < len; ++i)
    {
        maxnum = max(maxnum, num[i]);
    }
    return maxnum;
}
int main()
{
    int n; cin >> n;
    int prices[100005];
    int ans = 0;
    for (int i = 0; i < n; ++i) cin >> prices[i];
    for (int i = 0; i < n; ++i)
    {
        ans = max(max_num(prices, i, n), ans);
    }
    cout << ans;
    return 0;
}