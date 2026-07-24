#include <bits/stdc++.h>

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        std::unordered_map<int, int> m;
        auto const n = nums.size();
        for(int i = 0; i < n; ++i){
            auto diff = target - nums[i];
            auto it = m.find(diff);
            if(it == m.end()){
                m[nums[i]] = i;
            }
            else{
                return {it->second, i};
            }
        }
        return {};
    }
};

/* Key note:
- Re-implement the hash table in C if I have the time
*/