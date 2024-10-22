#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;
using ll = long long;
int a[100010],b[100010];
int main()
{
	int n, m; cin >> n >> m;
	for (int i = 1; i <= n; ++i)
	{
		cin >> a[i];
		b[i] = a[i] - a[i - 1];
	}
	for (int i = 1; i <= n; ++i) b

	int l, r, c;
	while (m--)
	{
		cin >> l >> r >> c;
		b[l] += c;
		b[r + 1] -= c;
	}
	for (int i = 1; i <= n; i++)
	{
		a[i] = a[i - 1] + b[i];
		cout << a[i] << " ";
	}
	return 0;
}


//const int N = 1010;
//int a[N][N], s[N][N];
//int main()
//{
//	int n, m, q; cin >> n >> m >> q;
//	for (int i = 1; i <= n; ++i)
//	{
//		for (int j = 1; j <= m; ++j)
//		{
//			cin >> a[i][j];
//		}
//	}
//	for (int i = 1; i <= n; ++i)
//	{
//		for (int j = 1; j <= m; ++j)
//		{
//			s[i][j] = s[i][j - 1] + s[i - 1][j] - s[i - 1][j - 1] + a[i][j];
//		}
//	}
//	int x1, y1, x2, y2;
//	while (q--)
//	{
//		cin >> x1 >> y1 >> x2 >> y2;
//		cout << s[x2][y2] - s[x1 - 1][y2] - s[x2][y1 - 1] + s[x1 - 1][y1 - 1] << endl;
//	}
//	return 0;
//}
////前缀和
//int a[100010];
//int main()
//{
//	int n, m; cin >> n >> m;
//	for (int i = 1; i <= n; ++i)cin >> a[i];
//	int s[100010];
//	s[0] = a[0];
//	for (int i = 1; i <= n; ++i)s[i] = s[i - 1] + a[i];
//	int l,r;
//	while (m--)
//	{
//		cin >> l >> r;
//		if (l == 0)
//			cout << s[r] << endl;
//		else
//			cout << s[r] - s[l - 1] << endl;
//	}
//}
//vector<int> div(vector<int>& A, int b, int& r)
//{
//	vector<int> C;
//	for (int i = A.size() - 1; i >= 0; --i)
//	{
//		r = 10 * r + A[i];
//		C.push_back(r / b);
//		r %= b;
//	}
//
//	reverse(C.begin(), C.end());
//
//	while (C.size() > 1 && C.back() == 0) C.pop_back();
//	return C;
//}
//int main()
//{
//	string a;
//	int b;
//	cin >> a >> b;
//	vector<int> A;
//	for (int i = a.size() - 1; i >= 0; --i)
//		A.push_back(a[i] - '0');
//	int r = 0;
//	auto C = div(A, b, r);
//	for (int i = C.size() - 1; i >= 0; --i)cout << C[i];
//	cout << endl << r << endl;
//	return 0;
//}
////高精度乘法
//vector<int> mul(vector<int>& A, int b)
//{
//	vector<int> C;
//	int t = 0;
//	for (int i = 0; i < A.size(); ++i)
//	{
//		t += A[i] * b;
//		C.push_back(t % 10);
//		t /= 10;
//	}		
//	if(t)
//		C.push_back(t);
//
//	while (C.size() > 1 && C.back() == 0)
//		C.pop_back();
//
//	return C;
//}

////浮点数二分
////立方根
//int main()
//{
//	double n;
//	cin >> n;
//	double l = -10000.0, r = 10000.0;
//	while (abs(l - r) > 1e-8)
//	{
//		double mid = (l + r) / 2;
//		if (mid * mid * mid < n)l = mid;
//		else r = mid;
//	}
//
//	printf("%.6lf", r);
//	return 0;
//}
//bool cmp(vector<int>& A, vector<int>& B)
//{
//	if (A.size() != B.size()) return A.size() > B.size();
//
//	for (int i = A.size() - 1; i >= 0; --i)
//	{
//		if(A[i] != B[i])
//			return A[i] > B[i];
//	}
//
//	return true;
//}
////高精度减法
//vector<int> sub(vector<int>& A, vector<int>B)
//{
//	vector<int> C;
//	int k = 0;
//	for (int i = 0; i < A.size(); ++i)
//	{
//		k = A[i] - k;
//		if (i < B.size())k -= B[i];
//		C.push_back((k + 10) % 10);
//		if (k < 0)k = 1;
//		else k = 0;
//	}
//	while (C.size() > 1 && C.back() == 0)
//		C.pop_back();
//
//	return C;
//}
//int main()
//{
//	string n, m;
//	cin >> n >> m;
//	vector<int> A, B;
//	for (int i = n.size() - 1; i >= 0; --i)A.push_back(n[i] - '0');
//	for (int i = m.size() - 1; i >= 0; --i)B.push_back(m[i] - '0');
//	vector<int> num;
//	if (cmp(A, B)) num = sub(A, B);
//	else
//	{
//		num = sub(B, A);
//		cout << '-';
//	}
//	for (int i = num.size() - 1; i >= 0; --i)cout << num[i];
//	return 0;
//}