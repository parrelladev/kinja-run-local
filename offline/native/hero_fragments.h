#pragma once
// Offline hero-fragment source approved by the user (26/11/2026). In 1.9.1 the
// fragments (HeroCfg.fragment_id: 701001/704002/704003) only came from online
// offers/events, so the offline build had no way to unlock heroes or raise
// stars. Local rule: every won run grants a fixed amount to one hero.
// Pure decision logic; the bridge reads HeroCfg/HeroStarCfg through IL2CPP.
#include <vector>

namespace kinja {

constexpr int kVictoryHeroFragments = 5;

struct FragmentHeroCandidate {
    int hero_id = 0;
    bool owned = false;
    bool receivable = false;   // HeroCfg.can_receive == 1 (only matters when not owned)
    bool star_capped = false;  // owned and no next HeroStarCfg row
    int fragment_id = 0;       // HeroCfg.fragment_id; <= 0 means unusable
};

// Order of `heroes`: configuration order (1001, 4002, 4003).
// 1) After the hero menu opens (Chapter 5), the first unowned receivable hero
//    collects fragments until it can be unlocked.
// 2) Otherwise the selected hero, if its next star exists.
// 3) Otherwise the first other owned hero that can still gain a star.
// Returns the index into `heroes`, or -1 when no hero can use fragments.
inline int choose_fragment_hero(const std::vector<FragmentHeroCandidate>& heroes, int selected_hero,
                                bool hero_menu_open) {
    if (hero_menu_open)
        for (size_t i = 0; i < heroes.size(); ++i)
            if (!heroes[i].owned && heroes[i].receivable && heroes[i].fragment_id > 0) return static_cast<int>(i);
    for (size_t i = 0; i < heroes.size(); ++i)
        if (heroes[i].hero_id == selected_hero && heroes[i].owned && !heroes[i].star_capped &&
            heroes[i].fragment_id > 0) return static_cast<int>(i);
    for (size_t i = 0; i < heroes.size(); ++i)
        if (heroes[i].owned && !heroes[i].star_capped && heroes[i].fragment_id > 0) return static_cast<int>(i);
    return -1;
}

}  // namespace kinja
