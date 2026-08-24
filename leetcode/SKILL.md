---
name: leetcode-helper
description: Use when the user is working on a leetcode/*.c problem in this repo — scaffolding a new problem file, or wants their existing solution checked, debugged, or explained.
---

## Hard rule: read-only on existing solutions

Once a `problem_N.c` file contains the user's own attempt (not just a fresh scaffold), it is **hands-off**. Never call Edit or Write on it, no matter how small or obviously correct the fix is — not even a one-character typo fix, and not even when the user says "check my code" or "fix it."

- You may Read the file, and use Bash to compile/run it (gcc, the existing test harness) to see what actually happens.
- When you find a bug, explain it: point at the line, say what's wrong, say why, and show the corrected line(s) as a snippet in your reply — never apply it yourself.
- Let the user paste the fix in. Then re-check by reading/compiling again on request.

This applies to every kind of help — debugging, optimization suggestions, style feedback. The user is doing the problem; you're the rubber duck, not the pair.

The one exception is scaffold creation (below), which only ever writes a *new* file that doesn't contain a solution yet.

## Creating a scaffold

Triggered when the user asks to scaffold/start a new problem (e.g. "write to leetcode problem N scaffold").

1. Find the next problem number if not given: `Glob leetcode/problem_*.c` and take the highest N not yet used.
2. Read one existing `problem_N.c` (pick a recent one) as the structural template — the shape is fixed across the whole folder:
   - includes (`stdio.h`, `stdlib.h`, `string.h` if strings involved, `stdbool.h`, `time.h`, the `_WIN32` block for `windows.h`/`psapi.h`)
   - a `/* Key notes: - TODO */` block above the solution
   - a `/* Problem N: <title> \n <leetcode URL> \n\n Input: ... \n\n Output: ... \n\n Examples: ... */` comment describing constraints, return semantics, and the problem statement's actual example inputs/outputs
   - the solution function itself, stubbed with a `// TODO` body and a placeholder return — never pre-write the algorithm, that's the user's part
   - the test harness: `TestCase` struct + `run_test`, using `strcpy` into a local buffer when the input is a mutable `char*`
   - the benchmark section: `current_working_set_bytes`, `high_res_ms`, `benchmark()` — copy this boilerplate verbatim, only changing the sample input and iteration comment
   - `main()` wiring up `run_test` calls for the problem's examples/edge cases, then `benchmark()`
3. Fill in the problem statement (number, title, URL, Input/Output, Examples) from what the user gives you or from the known LeetCode problem, including the problem page's actual example inputs/outputs in the header comment; leave the function body as a stub.
4. In `main()`, wire up `run_test` calls using the actual examples from the LeetCode problem statement (not invented ones) — same inputs/outputs as shown on the problem page. Add a couple of obvious edge cases on top if the problem statement doesn't already cover them (e.g. smallest valid input, all-same-element input).
5. Write the new file. Compile it once (`gcc problem_N.c -o problem_N.exe`) to confirm it builds — a stub returning a placeholder will fail the tests, that's expected; just confirm no compile errors.
