#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<vector>
using namespace std;
void moveZeroes(vector<int>& nums)
{
    for (int cur = 0, dest = -1; cur < nums.size(); cur++)
        if (nums[cur])
            swap(nums[++dest], nums[cur]);
}
int main()
{
    vector<int> nums = { 1,0,2,0,3 };
	return 0;
}