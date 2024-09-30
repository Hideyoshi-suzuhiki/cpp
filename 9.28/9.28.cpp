#define _CRT_SECURE_NO_WARNINGS
#include<vector>
#include<iostream>
using namespace std;
void moveZeroes(vector<int>& nums)
{

}
int main()
{
    vector<int> nums = { 0,0,0,0,0,0};
    int fast = 0, slow = 0;
    while (fast < int(nums.size() - 1))
    {
        do fast++; 
        while (nums[fast] == 0);
        do slow++; 
        while (nums[slow] != 0);
        if (fast > slow) swap(nums[fast], nums[slow]);
    }
    for (int i = 0; i < nums.size(); ++i)
    {
        cout << nums[i];
    }
    return 0;
}