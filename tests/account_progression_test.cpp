#include "../offline/native/account_progression.h"
#include "account_cost_fixture.h"
#include <cassert>
#include <cstdio>

int main() {
    kinja::AccountProgress p{};
    assert(costs.size() == 200 && costs[0] == 1250 && costs[1] == 2000 && costs[2] == 3000);
    // Reported screenshot: use only its already-earned EXP, preserving remainder.
    assert(kinja::advance_account(2, 5838, costs, p) && p.level == 4 && p.exp == 838);
    assert(kinja::advance_account(p.level, p.exp, costs, p) && p.level == 4 && p.exp == 838);
    for (int level = 1; level < 200; ++level) {
        assert(kinja::advance_account(level, costs[level - 1] - 1, costs, p));
        assert(p.level == level && p.exp == costs[level - 1] - 1);
        assert(kinja::advance_account(level, costs[level - 1], costs, p));
        assert(p.level == level + 1 && p.exp == 0);
    }
    // New reward crossing a boundary; existing manually promoted level is retained.
    assert(kinja::advance_account(1, 1240 + 30, costs, p) && p.level == 2 && p.exp == 20);
    assert(kinja::advance_account(2, 0, costs, p) && p.level == 2 && p.exp == 0);
    assert(kinja::advance_account(200, INT32_MAX, costs, p) && p.level == 200 && p.exp == INT32_MAX);
    assert(!kinja::advance_account(0, 0, costs, p));
    assert(!kinja::advance_account(201, 0, costs, p));
    assert(!kinja::advance_account(1, -1, costs, p));
    assert(!kinja::advance_account(1, int64_t(INT32_MAX) + 1, costs, p));
    assert(!kinja::advance_account(1, 0, {}, p));
    auto invalid = costs;
    invalid[1] = 0;
    assert(!kinja::advance_account(1, 5000, invalid, p));
    std::puts("PASS: screenshot 2/5838 -> 4/838, idempotence, all 199 boundaries, new EXP, legacy level, cap, invalid inputs");
}
