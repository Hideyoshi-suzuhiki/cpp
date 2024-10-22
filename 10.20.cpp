#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<vector>
using namespace std;
//int a[100010];
//int num = 0;
//
//int bsearch_1(int l, int r)
//{
//	while (l < r)
//	{
//		int mid = l + r + 1 >> 1;
//		if (a[mid] <= num)l = mid;
//		else r = mid - 1;
//	}
//	return l;
//}
//
//int bsearch_2(int l, int r)
//{
//	while (l < r)
//	{
//		int mid = l + r >> 1;
//		if (a[mid] >= num)r = mid;
//		else l = mid + 1;
//	}
//	return l;
//}
//int main()
//{
//	int n, q; cin >> n >> q;
//	for (int i = 0; i < n; ++i) cin >> a[i];
//	while (q--)
//	{
//		cin >> num;
//		if (a[bsearch_1(0, n - 1)] == num && a[bsearch_2(0, n - 1)] == num)
//		{
//			cout << bsearch_1(0, n - 1) << ' ' << bsearch_2(0, n - 1);
//		}
//		else
//			cout << "-1 -1";
//	}
//	return 0;
//}
//int a[100005],tmp[100005];
//long long ans = 0;
//void merge_sort(int a[], int l, int r)
//{
//	if (l >= r) return;
//	int mid = (l + r) / 2;
//	merge_sort(a, l, mid);
//	merge_sort(a, mid + 1, r);
//
//	int i = l, j = mid + 1,k = 0;
//	while (i <= mid && j <= r)
//	{
//		if (a[i] <= a[j])tmp[k++] = a[i++];
//		else
//		{
//			tmp[k++] = a[j++];
//			ans += (mid - i + 1);
//		}
//	}
//	while(i <= mid) tmp[k++] = a[i++];
//	while(j <= r) tmp[k++] = a[j++];
//	for (int i = l, j = 0; i <= r; i++, j++)a[i] = tmp[j];
//}
//int main()
//{
//	int n; cin >> n;
//	for (int i = 0; i < n; i++)
//	merge_sort(a, 0, n - 1);
//	for (int i = 0; i < n; i++) printf("%d ", a[i]);
//	cout << ans;
//	return 0;
//}
vector<int> add(vector<int> &A, vector<int> &B)
{
	vector<int> C;
	int t = 0;
	for (int i = 0; i < A.size() || i < B.size(); i++)
	{
		if (i < A.size()) t += A[i];
		if (i < B.size()) t += B[i];
		C.push_back(t % 10);
		t /= 10;
	}

	if (t)C.push_back(1);
	
	return C;
}
vector<int> A, B;
int main()
{
	string a, b;
	cin >> a >> b;
	for (int i = a.length() - 1; i >= 0;--i)A.push_back(a[i] - '0');
	for (int i = b.length() - 1; i >= 0;--i)B.push_back(b[i] - '0');
	vector<int> C = add(A, B);
	for (int i = C.size() - 1; i >= 0; --i) cout << C[i];
	return 0;
}