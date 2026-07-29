#include <algorithm>
#include <cstdio>
#include <memory>
#include <random>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: the "cost of virtual functions" people worry about is
 * usually not the vtable indirection itself -- it's the branch the CPU
 * has to predict to know which override it's about to jump to. If you
 * call the same 10,000 Dogs then the same 10,000 Cats in a row, the
 * branch predictor learns the pattern easily. If you shuffle them so
 * the concrete type is unpredictable call-to-call, you pay a real
 * misprediction penalty on every single call, on every compiler --
 * unlike the sorted/unsorted case, no compiler can optimize this away
 * because it's genuine runtime polymorphism.
 */

struct Mammal {
    virtual ~Mammal() = default;
    virtual int number() const = 0;
};
struct Dog : Mammal {
    int number() const override { return 1; }
};
struct Cat : Mammal {
    int number() const override { return 2; }
};

static long long sum_numbers(const std::vector<std::unique_ptr<Mammal>>& animals) {
    long long sum = 0;
    for (auto& m : animals) sum += m->number();
    return sum;
}

int main() {
    const int per_kind = 10000;
    std::vector<std::unique_ptr<Mammal>> grouped;
    grouped.reserve(per_kind * 2);
    for (int i = 0; i < per_kind; ++i) grouped.push_back(std::make_unique<Dog>());
    for (int i = 0; i < per_kind; ++i) grouped.push_back(std::make_unique<Cat>());

    std::vector<std::unique_ptr<Mammal>> shuffled;
    shuffled.reserve(per_kind * 2);
    for (int i = 0; i < per_kind; ++i) shuffled.push_back(std::make_unique<Dog>());
    for (int i = 0; i < per_kind; ++i) shuffled.push_back(std::make_unique<Cat>());
    std::mt19937 rng(7);
    std::shuffle(shuffled.begin(), shuffled.end(), rng);

    int iterations = 500;
    double grouped_ms = time_ms([&] { do_not_optimize(sum_numbers(grouped)); }, iterations);
    double shuffled_ms = time_ms([&] { do_not_optimize(sum_numbers(shuffled)); }, iterations);

    printf("grouped:  %.5f ms\n", grouped_ms);
    printf("shuffled: %.5f ms\n", shuffled_ms);
    printf("ratio:    %.2fx\n", shuffled_ms / grouped_ms);

    printf("\nExpect shuffled noticeably slower on every compiler --\n");
    printf("this is branch misprediction on the vtable jump, not the\n");
    printf("indirection itself.\n");

    return 0;
}
