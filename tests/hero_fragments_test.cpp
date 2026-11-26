#include "../offline/native/hero_fragments.h"
#include <cassert>
#include <cstdio>

using namespace kinja;

int main() {
    assert(kVictoryHeroFragments == 5);
    // User's save: only 1001 owned/selected, menu closed before Chapter 5.
    std::vector<FragmentHeroCandidate> h{
        {1001, true, true, false, 701001}, {4002, false, true, false, 704002}, {4003, false, true, false, 704003}};
    assert(choose_fragment_hero(h, 1001, false) == 0);
    // Menu open (Chapter 5 cleared): unlock 4002 first, then 4003.
    assert(choose_fragment_hero(h, 1001, true) == 1);
    h[1].owned = true;
    assert(choose_fragment_hero(h, 1001, true) == 2);
    h[2].owned = true;
    // All owned: selected hero gets them.
    assert(choose_fragment_hero(h, 4002, true) == 1);
    // Selected at star cap: first other hero that can still grow.
    h[1].star_capped = true;
    assert(choose_fragment_hero(h, 4002, true) == 0);
    h[0].star_capped = h[2].star_capped = true;
    assert(choose_fragment_hero(h, 4002, true) == -1);
    // A hero that cannot be received is never targeted for unlocking.
    std::vector<FragmentHeroCandidate> locked{{1001, true, true, false, 701001}, {4002, false, false, false, 704002}};
    assert(choose_fragment_hero(locked, 1001, true) == 0);
    // Missing fragment id is unusable.
    std::vector<FragmentHeroCandidate> broken{{1001, true, true, false, 0}};
    assert(choose_fragment_hero(broken, 1001, true) == -1);
    std::puts("PASS: victory hero fragment target selection");
}
