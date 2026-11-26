#pragma once
// Pure planner for configured chapter clear rewards (first win and progress
// milestones). It has no IL2CPP or Android dependency so it can run in host
// tests. The bridge reads GameRegion/Reward rows from the original 1.9.1
// tables and only asks this header which slots are still unclaimed.
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace kinja {

// Slot 0 is GameRegion.first_win_reward; slot n (1..98) is progress_reward[n-1].
constexpr int kChapterMaxSlot = 98;
constexpr int kChapterLedgerVersion = 1;

struct RewardGrant { int item_id; int count; };

inline bool chapter_claim_key(int region, int slot, int& key) {
    if (region <= 0 || region > INT32_MAX / 100 - 1 || slot < 0 || slot > kChapterMaxSlot) return false;
    key = region * 100 + slot;
    return true;
}

inline bool parse_positive_int(const std::string& text, size_t begin, size_t end, int& value) {
    if (begin >= end || end - begin > 10) return false;
    int64_t result = 0;
    for (size_t i = begin; i < end; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
        result = result * 10 + (text[i] - '0');
    }
    if (result <= 0 || result > INT32_MAX) return false;
    value = static_cast<int>(result);
    return true;
}

// Fixed reward entries in the 1.9.1 Reward table use "ItemId_Count"
// (RewardCfg 10013 = "40001_2"). Weighted pools use three parts
// ("ItemId_Weight_Count") and are deliberately refused here.
inline bool parse_fixed_reward_token(const std::string& token, RewardGrant& grant) {
    const size_t sep = token.find('_');
    if (sep == std::string::npos || token.find('_', sep + 1) != std::string::npos) return false;
    return parse_positive_int(token, 0, sep, grant.item_id) &&
           parse_positive_int(token, sep + 1, token.size(), grant.count);
}

inline bool merge_reward_grant(std::vector<RewardGrant>& grants, const RewardGrant& grant) {
    if (grant.item_id <= 0 || grant.count <= 0) return false;
    for (auto& existing : grants) {
        if (existing.item_id != grant.item_id) continue;
        if (existing.count > INT32_MAX - grant.count) return false;
        existing.count += grant.count;
        return true;
    }
    grants.push_back(grant);
    return true;
}

struct ChapterLedger { std::set<int> claims; };

// Strict reader for {"Version":1,"Claims":[200,201,...]} (whitespace allowed).
inline bool parse_chapter_ledger(const std::string& json, ChapterLedger& ledger) {
    size_t i = 0;
    auto ws = [&]() { while (i < json.size() && std::isspace(static_cast<unsigned char>(json[i]))) ++i; };
    auto take = [&](char c) { ws(); if (i < json.size() && json[i] == c) { ++i; return true; } return false; };
    auto key = [&](const char* name) {
        ws();
        const std::string quoted = std::string("\"") + name + "\"";
        if (json.compare(i, quoted.size(), quoted) != 0) return false;
        i += quoted.size();
        return take(':');
    };
    auto number = [&](int& value) {
        ws();
        size_t start = i;
        while (i < json.size() && std::isdigit(static_cast<unsigned char>(json[i]))) ++i;
        return parse_positive_int(json, start, i, value);
    };
    ChapterLedger parsed;
    int version = 0;
    if (!take('{') || !key("Version") || !number(version) || version != kChapterLedgerVersion ||
        !take(',') || !key("Claims") || !take('[')) return false;
    if (!take(']')) {
        do {
            int claim = 0;
            if (!number(claim) || claim < 100 || claim % 100 > kChapterMaxSlot ||
                !parsed.claims.insert(claim).second) return false;
        } while (take(','));
        if (!take(']')) return false;
    }
    if (!take('}')) return false;
    ws();
    if (i != json.size()) return false;
    ledger = parsed;
    return true;
}

inline std::string serialize_chapter_ledger(const ChapterLedger& ledger) {
    std::string out = "{\"Version\":" + std::to_string(kChapterLedgerVersion) + ",\"Claims\":[";
    bool first = true;
    for (int claim : ledger.claims) {
        if (!first) out += ",";
        out += std::to_string(claim);
        first = false;
    }
    return out + "]}\n";
}

// Saves created before the ledger existed already received exactly these
// hard-coded rewards: region 2 (Chapter 1) first-win bronze key 100001 and its
// single 100% milestone (Reward 10013 = 40001_2), and region 3 (Chapter 2)
// first-win silver key 100002 (also restored by repair_saved_chapter2_rewards).
// Never grant these slots again when migrating such a save.
inline void add_legacy_chapter_claims(const std::vector<int>& passed_regions, ChapterLedger& ledger) {
    int key = 0;
    if (std::find(passed_regions.begin(), passed_regions.end(), 2) != passed_regions.end()) {
        if (chapter_claim_key(2, 0, key)) ledger.claims.insert(key);
        if (chapter_claim_key(2, 1, key)) ledger.claims.insert(key);
    }
    if (std::find(passed_regions.begin(), passed_regions.end(), 3) != passed_regions.end() &&
        chapter_claim_key(3, 0, key)) ledger.claims.insert(key);
}

struct ChapterSlotRewards {
    int slot = 0;
    bool resolved = false;            // false: config/type not understood; retry later
    std::vector<RewardGrant> grants;  // empty + resolved means "nothing configured"
};

struct ChapterSettlement {
    std::vector<RewardGrant> grants;
    std::vector<int> new_claims;
    int unresolved_slots = 0;
};

// Claims every resolved, unclaimed slot of a cleared region. Unresolved slots
// stay unclaimed so a later build/bootstrap can grant them without duplicates.
inline bool plan_chapter_settlement(int region, const std::vector<ChapterSlotRewards>& slots,
                                    const ChapterLedger& ledger, ChapterSettlement& out) {
    ChapterSettlement result;
    std::set<int> seen;
    for (const auto& slot : slots) {
        int key = 0;
        if (!chapter_claim_key(region, slot.slot, key) || !seen.insert(slot.slot).second) return false;
        if (ledger.claims.count(key)) continue;
        if (!slot.resolved) { ++result.unresolved_slots; continue; }
        for (const auto& grant : slot.grants)
            if (!merge_reward_grant(result.grants, grant)) return false;
        result.new_claims.push_back(key);
    }
    out = result;
    return true;
}

}  // namespace kinja
