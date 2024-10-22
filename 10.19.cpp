#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
using namespace std;
int a[100005];
void quick_sort(int a[], int l, int r)
{
	if (l >= r) return;
	int mid = a[(l + r) / 2];
	int i = l - 1, j = r + 1;
	while (i < j)
	{
		do i++; while (a[i] < mid);
		do j--; while (a[j] > mid);
		if (i < j) swap(a[i], a[j]);
	}
	quick_sort(a, l, j);
	quick_sort(a, j + 1, r);
}
int main()
{
	int n,k; cin >> n >> k;
	for (int i = 0; i < n; ++i) cin >> a[i];
	quick_sort(a, 0, n - 1);
	cout << a[k - 1];
	return 0;
}