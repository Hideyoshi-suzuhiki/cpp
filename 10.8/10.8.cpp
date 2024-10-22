#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main()
{
    int n;
    cin >> n;
    vector<int> num(n);
    for (int i = 0; i < n; i++) cin >> num[i];
    int max_gcd = 0;
    sort(num.begin(), num.end(), greater<int>());
    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (num[i] % num[j] == 0)
                max_gcd = max(max_gcd, num[j]);
        }
    }
    cout << max_gcd;
	return 0;
}