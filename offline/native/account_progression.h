#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace kinja {
struct AccountProgress { int level; int exp; };

// EXP is the unspent balance at the saved level, not lifetime EXP.
// The last configured level is the cap; retain excess EXP there.
inline bool advance_account(int level, int64_t exp, const std::vector<int>& costs,
                            AccountProgress& result) {
    if (level < 1 || static_cast<std::size_t>(level) > costs.size() || exp < 0 || exp > INT32_MAX)
        return false;
    for (int cost : costs) if (cost <= 0) return false;
    while (static_cast<std::size_t>(level) < costs.size() && exp >= costs[level - 1]) {
        exp -= costs[level - 1];
        ++level;
    }
    result = {level, static_cast<int>(exp)};
    return true;
}
}
