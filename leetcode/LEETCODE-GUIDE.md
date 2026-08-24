# LeetCode Pattern

## 1. Identification Key
Constraints (n):
- n ≤ 20 → exponential ok: brute force, backtracking
- n 10^3-10^6 → O(n) / O(n log n): greedy, two pointers, heap, DP
- n ≥ 10^7 → O(log n) / O(1): binary search, math

Input shape:
- Tree/BST → DFS (paths), BFS (level order)
- Graph → BFS (shortest path), DFS (components), union find, topo sort
- 2D grid → DFS/BFS (islands), union find, DP (paths)
- Sorted array → two pointers, binary search, greedy
- String → two pointers (palindrome), sliding window (substring), trie, stack (brackets)
- Linked list → fast/slow pointers, dummy node, cycle detection

Output shape:
- List of lists → backtracking
- Single number → DP, greedy, math
- Modified array in-place → two pointers
- Ordered list → sort/comparator, topo sort, heap

Keywords:
- DP → number of ways, max/min sum/profit/cost, can you reach, longest/shortest subsequence
- Two pointers → palindrome, sorted array, target sum, remove duplicates
- Sliding window → substring/subarray, fixed/variable window, contains all
- Heap → k largest/smallest, top k, median
- Stack → parentheses, valid expression, next greater/smaller
- HashMap → count frequency, find duplicates, anagram
- Union find → connected components, number of groups
- Binary search → kth element, minimize the maximum, first/last occurrence
- Bit manipulation → XOR, single number, power of two

## 2. Core Patterns
### Two Pointers — O(n)
- 125 Valid Palindrome (E)
- 167 Two Sum II (E)
- 15 3Sum (M)
- 16 3Sum Closest (M)
- 18 4Sum (M)
- 11 Container With Most Water (M)
- 5 Longest Palindromic Substring (M)
- 42 Trapping Rain Water (H)
- 632 Smallest Range Covering K Lists (H)

### Sliding Window — O(n)
- 3 Longest Substring Without Repeating Chars (M)
- 567 Permutation in String (M)
- 424 Longest Repeating Char Replacement (M)
- 209 Minimum Size Subarray Sum (M)
- 438 Find All Anagrams in a String (M)
- 76 Minimum Window Substring (H)
- 239 Sliding Window Maximum (H)
- 30 Substring w/ Concatenation of All Words (H)

### Binary Search — O(log n)
- 704 Binary Search (E)
- 74 Search a 2D Matrix (M)
- 34 First/Last Position in Sorted Array (M)
- 153 Find Min in Rotated Sorted Array (M)
- 33 Search in Rotated Sorted Array (M)
- 875 Koko Eating Bananas (M)
- 4 Median of Two Sorted Arrays (H)
- 410 Split Array Largest Sum (H)
- 719 Kth Smallest Pair Distance (H)

### BFS — O(V+E)
- 102 Binary Tree Level Order Traversal (M)
- 133 Clone Graph (M)
- 542 01 Matrix (M)
- 1091 Shortest Path in Binary Matrix (M)
- 994 Rotting Oranges (M)
- 127 Word Ladder (H)
- 1345 Jump Game IV (H)
- 815 Bus Routes (H)

### DFS — O(V+E)
- 543 Diameter of Binary Tree (E)
- 200 Number of Islands (M)
- 130 Surrounded Regions (M)
- 417 Pacific Atlantic Water Flow (M)
- 236 Lowest Common Ancestor (M)
- 207 Course Schedule (M)
- 124 Binary Tree Max Path Sum (H)
- 329 Longest Increasing Path in a Matrix (H)
- 1192 Critical Connections in a Network (H)

### Backtracking — O(2^n) / O(n!)
- 78 Subsets (M)
- 46 Permutations (M)
- 47 Permutations II (M)
- 22 Generate Parentheses (M)
- 39 Combination Sum (M)
- 79 Word Search (M)
- 51 N-Queens (H)
- 37 Sudoku Solver (H)
- 301 Remove Invalid Parentheses (H)

### Heap / Priority Queue — O(log n) push/pop
- 215 Kth Largest Element (M)
- 347 Top K Frequent Elements (M)
- 973 K Closest Points to Origin (M)
- 621 Task Scheduler (M)
- 253 Meeting Rooms II (M)
- 23 Merge k Sorted Lists (H)
- 295 Find Median from Data Stream (H)
- 407 Trapping Rain Water II (H)
- 218 The Skyline Problem (H)

### Dynamic Programming — O(n)-O(n^2)
- 70 Climbing Stairs (E)
- 198 House Robber (M)
- 322 Coin Change (M)
- 139 Word Break (M)
- 416 Partition Equal Subset Sum (M)
- 62 Unique Paths (M)
- 300 Longest Increasing Subsequence (M)
- 1143 Longest Common Subsequence (M)
- 72 Edit Distance (H)
- 10 Regular Expression Matching (H)
- 44 Wildcard Matching (H)
- 312 Burst Balloons (H)
- 115 Distinct Subsequences (H)

## 3. Minor Patterns
### Stack / Monotonic stack
keywords: parentheses, next greater element
- 20 Valid Parentheses (E)
- 739 Daily Temperatures (M)
- 84 Largest Rectangle in Histogram (H)
- 224 Basic Calculator (H)

### HashMap
keywords: count frequency, anagram
- 1 Two Sum (E)
- 49 Group Anagrams (M)
- 128 Longest Consecutive Sequence (M)
- 336 Palindrome Pairs (H)

### Trie
keywords: word prefixes, word search
- 208 Implement Trie (M)
- 212 Word Search II (H)
- 1032 Stream of Characters (H)

### Greedy
keywords: minimum operations
- 55 Jump Game (M)
- 435 Non-overlapping Intervals (M)
- 763 Partition Labels (M)
- 135 Candy (H)

### Union Find
keywords: connected components, number of groups
- 547 Number of Provinces (M)
- 261 Graph Valid Tree (M)
- 1319 Operations to Make Network Connected (M)
- 685 Redundant Connection II (H)

### Bit manipulation
keywords: XOR, single number, power of two
- 136 Single Number (E)
- 191 Number of 1 Bits (E)
- 371 Sum of Two Integers (M)
- 1707 Max XOR With Element From Array (H)

### Topological sort
keywords: dependencies, valid task order
- 210 Course Schedule II (M)
- 269 Alien Dictionary (H)
- 1857 Largest Color Value in a Directed Graph (H)

### Math / Geometry
keywords: coordinate, prime numbers
- 48 Rotate Image (M)
- 54 Spiral Matrix (M)
- 149 Max Points on a Line (H)
- 587 Erect the Fence (H)