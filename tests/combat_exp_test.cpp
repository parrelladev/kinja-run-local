#include "../offline/native/combat_exp.h"
#include "../offline/native/account_progression.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main() {
    assert(kinja::combat_exp_for_kill(false, false) == 5);
    assert(kinja::combat_exp_for_kill(false, true) == 25);
    assert(kinja::combat_exp_for_kill(true, false) == 50);
    assert(kinja::combat_exp_for_kill(true, true) == 50);

    int delta = -1;
    // The win gate is in the bridge; no settlement call occurs on a loss.
    assert(kinja::combat_exp_to_credit(80, 0, delta) && delta == 80);
    assert(kinja::combat_exp_to_credit(80, 80, delta) && delta == 0);
    assert(kinja::combat_exp_to_credit(85, 80, delta) && delta == 5);
    assert(!kinja::combat_exp_to_credit(80, 85, delta));
    assert(!kinja::combat_exp_to_credit(-1, 0, delta));
    assert(!kinja::combat_exp_to_credit(int64_t(INT32_MAX) + 1, 0, delta));

    kinja::AccountProgress progress{};
    const std::vector<int> costs{1250, 2000, 3000};
    assert(kinja::advance_account(2, 1990 + 5 + 25 + 50, costs, progress));
    assert(progress.level == 3 && progress.exp == 70);
    std::puts("PASS: offline kill EXP, one-time and late deltas, level boundary");
}
