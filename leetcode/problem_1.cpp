#include <bits/stdc++.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Problem 1: Two Sum
 * https://leetcode.com/problems/two-sum/
 *
 * Input:
 *   nums   - array of integers, 2 <= nums.length <= 10^4, -10^9 <= nums[i] <= 10^9
 *   target - integer, -10^9 <= target <= 10^9
 *   Exactly one valid answer is guaranteed to exist; may not use the same element twice.
 *
 * Output:
 *   Vector of 2 indices [i, j] such that nums[i] + nums[j] == target.
 */

class Solution {
public:
    std::vector<int> twoSum(std::vector<int>& nums, int target) {
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

/* ---------- test harness ---------- */

struct TestCase {
    std::string name;
    std::vector<int> nums;
    int target;
    std::vector<int> expected;
};

static void run_test(Solution& sol, const TestCase& tc) {
    auto nums = tc.nums; // twoSum takes a non-const reference
    auto result = sol.twoSum(nums, tc.target);

    bool pass = result.size() == 2 &&
                ((result[0] == tc.expected[0] && result[1] == tc.expected[1]) ||
                 (result[0] == tc.expected[1] && result[1] == tc.expected[0]));

    std::cout << "[" << tc.name << "] target=" << tc.target << " -> got [";
    for (size_t i = 0; i < result.size(); ++i) std::cout << result[i] << (i + 1 < result.size() ? ", " : "");
    std::cout << "], expected [" << tc.expected[0] << ", " << tc.expected[1] << "] -> "
              << (pass ? "PASS" : "FAIL") << std::endl;
}

/* ---------- benchmark ---------- */

static size_t current_working_set_bytes() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#else
    return 0;
#endif
}

static void benchmark(Solution& sol) {
    int n = 20000;
    std::vector<int> nums(n);
    for (int i = 0; i < n; ++i) nums[i] = i * 2; // no two elements sum to an odd target
    int target = nums[n - 2] + nums[n - 1];      // force worst case: match found at the end

    size_t before = current_working_set_bytes();
    auto start = std::chrono::high_resolution_clock::now();
    auto result = sol.twoSum(nums, target);
    auto end = std::chrono::high_resolution_clock::now();
    size_t after = current_working_set_bytes();

    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "\n[benchmark] n=" << n << ", elapsed=" << elapsed_ms << " ms" << std::endl;
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    std::cout << "Working set delta (twoSum call only): " << delta_kb << " KB" << std::endl;
    std::cout << "Process working set (whole program, for reference): " << after / (1024.0 * 1024.0) << " MB" << std::endl;
#else
    std::cout << "Memory benchmark only implemented for Windows in this file." << std::endl;
#endif
}

int main() {
    Solution sol;

    run_test(sol, {"example 1", {2, 7, 11, 15}, 9, {0, 1}});
    run_test(sol, {"example 2", {3, 2, 4}, 6, {1, 2}});
    run_test(sol, {"duplicate values", {3, 3}, 6, {0, 1}});
    run_test(sol, {"negative numbers", {-1, -2, -3, -4, -5}, -8, {2, 4}});
    run_test(sol, {"zero values", {0, 4, 3, 0}, 0, {0, 3}});

    benchmark(sol);

    return 0;
}

/* Key note:
- Re-implement the hash table in C if I have the time
*/
