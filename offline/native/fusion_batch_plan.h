#pragma once
#include <algorithm>
#include <array>
#include <unordered_set>
#include <vector>

// User-authorized offline selection policy; not a recovered ET server algorithm.
struct FusionBatchItem {
    int uid = 0, id = 0, quality = 0, level = 0, part = 0;
    bool worn = false, removed = false, changed = false;
};
struct FusionBatchStep {
    FusionBatchItem root;
    std::array<FusionBatchItem, 2> materials;
};
inline bool plan_fusion_batch(std::vector<FusionBatchItem>& items,
                              std::vector<FusionBatchStep>& steps) {
    steps.clear();
    std::unordered_set<int> uids;
    for (const auto& item : items)
        if (item.uid <= 0 || item.id <= 0 || item.quality < 1 || item.quality > 15 ||
            item.level < 1 || item.part < 1 || item.removed || item.changed ||
            !uids.insert(item.uid).second) return false;
    for (int quality = 1; quality <= 3; ++quality) {
        for (;;) {
            std::vector<size_t> group;
            for (size_t i = 0; i < items.size(); ++i) {
                const auto& candidate = items[i];
                if (candidate.removed || candidate.worn || candidate.quality != quality) continue;
                group.clear();
                for (size_t j = 0; j < items.size(); ++j)
                    if (!items[j].removed && !items[j].worn && items[j].quality == quality &&
                        items[j].id == candidate.id && items[j].part == candidate.part) group.push_back(j);
                if (group.size() >= 3) break;
            }
            if (group.size() < 3) break;
            std::sort(group.begin(), group.end(), [&](size_t a, size_t b) {
                return items[a].level != items[b].level ? items[a].level > items[b].level :
                       items[a].uid < items[b].uid;
            });
            const size_t root = group.front(), m1 = group[group.size() - 1], m2 = group[group.size() - 2];
            steps.push_back({items[root], {items[m1], items[m2]}});
            items[root].quality++;
            items[root].changed = true;
            items[m1].removed = items[m2].removed = true;
        }
    }
    return true;
}
