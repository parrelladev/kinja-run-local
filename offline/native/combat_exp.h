#pragma once
#include <cstdint>
#include <climits>

namespace kinja {
// Explicit offline approximation; the original 1.9.1 server formula is unknown.
constexpr int combat_exp_for_kill(bool boss, bool elite) {
    return boss ? 50 : (elite ? 25 : 5);
}

inline bool combat_exp_to_credit(int64_t total, int64_t credited, int& delta) {
    if (total < 0 || credited < 0 || credited > total || total - credited > INT32_MAX)
        return false;
    delta = static_cast<int>(total - credited);
    return true;
}
}
