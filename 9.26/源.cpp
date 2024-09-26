#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<algorithm>
using namespace std;

//int fib(int n)
//{
//	if (n == 1)
//		return 1;
//	else if (n == 0)
//		return 0;
//	else if (n == 2)
//		return 1;
//	else
//		return fib(n - 1) + fib(n - 2);
//}
//int main()
//{
//	int n,ans = 100000; cin >> n;
//	for (int i = 0; i < 26; ++i)
//	{
//		ans = min(ans, int(abs(n - fib(i))));
//	}
//	cout << ans;
//	return 0;
//}
#include<vector>
int main()
{
	int n; cin >> n;
	int vec[32][32];
	for (int i = 0; i < n; ++i)
	{
		int j = i;
		while (j >= 0)
		{
			if (i != 0 && j != i && j != 0)
				vec[i][j] = vec[i - 1][j - 1] + vec[i - 1][j];
			else
				vec[i][j] = 1;
			j--;
		}
	}
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j <= i; j++)
		{
			printf("%-5d", vec[i][j]);
		}
		cout << endl;
	}
	return 0;
}
