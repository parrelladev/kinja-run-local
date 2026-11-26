#include "../offline/native/chapter_rewards.h"
#include <cassert>
#include <cstdio>
#include <vector>

using namespace kinja;

static ChapterSlotRewards slot(int n, std::vector<RewardGrant> grants, bool resolved = true) {
    ChapterSlotRewards s;
    s.slot = n;
    s.resolved = resolved;
    s.grants = grants;
    return s;
}

int main() {
    RewardGrant g{};
    assert(parse_fixed_reward_token("40001_2", g) && g.item_id == 40001 && g.count == 2);
    assert(parse_fixed_reward_token("102_50", g) && g.item_id == 102 && g.count == 50);
    assert(!parse_fixed_reward_token("20001_48_1", g));   // weighted pool row
    assert(!parse_fixed_reward_token("40001", g));
    assert(!parse_fixed_reward_token("40001_0", g));
    assert(!parse_fixed_reward_token("0_1", g));
    assert(!parse_fixed_reward_token("-1_1", g));
    assert(!parse_fixed_reward_token("40001_2x", g));
    assert(!parse_fixed_reward_token("_2", g));
    assert(!parse_fixed_reward_token("99999999999_1", g));

    int key = 0;
    assert(chapter_claim_key(13, 0, key) && key == 1300);
    assert(chapter_claim_key(13, 2, key) && key == 1302);
    assert(!chapter_claim_key(0, 0, key));
    assert(!chapter_claim_key(5, 99, key));

    ChapterLedger ledger;
    assert(parse_chapter_ledger("{\"Version\":1,\"Claims\":[]}", ledger) && ledger.claims.empty());
    assert(parse_chapter_ledger(" {\"Version\": 1, \"Claims\": [200, 201,300]}\n", ledger));
    assert(ledger.claims.size() == 3 && ledger.claims.count(201));
    assert(parse_chapter_ledger(serialize_chapter_ledger(ledger), ledger) && ledger.claims.size() == 3);
    assert(!parse_chapter_ledger("{\"Version\":2,\"Claims\":[]}", ledger));
    assert(!parse_chapter_ledger("{\"Version\":1,\"Claims\":[200,200]}", ledger));
    assert(!parse_chapter_ledger("{\"Version\":1,\"Claims\":[99]}", ledger));
    assert(!parse_chapter_ledger("{\"Version\":1,\"Claims\":[299]}", ledger));
    assert(!parse_chapter_ledger("{\"Version\":1,\"Claims\":[200]} x", ledger));
    assert(!parse_chapter_ledger("", ledger));

    // Migration of the user's save shape: regions 2..12 passed with the old build.
    ChapterLedger migrated;
    std::vector<int> passed;
    for (int r = 2; r <= 12; ++r) passed.push_back(r);
    add_legacy_chapter_claims(passed, migrated);
    assert(migrated.claims == (std::set<int>{200, 201, 300}));

    // Region 2 already settled by the legacy build: nothing new.
    ChapterSettlement plan;
    assert(plan_chapter_settlement(2, {slot(0, {{100001, 1}}), slot(1, {{40001, 2}})}, migrated, plan));
    assert(plan.grants.empty() && plan.new_claims.empty() && plan.unresolved_slots == 0);

    // Region 3: first win already given; its milestone is still owed.
    assert(plan_chapter_settlement(3, {slot(0, {{100002, 1}}), slot(1, {{40001, 1}, {102, 30}})}, migrated, plan));
    assert(plan.grants.size() == 2 && plan.grants[0].item_id == 40001 && plan.grants[0].count == 1);
    assert(plan.new_claims == (std::vector<int>{301}));

    // Region 4: everything owed; equal item IDs merge.
    assert(plan_chapter_settlement(4, {slot(0, {{100001, 1}}), slot(1, {{40001, 1}}), slot(2, {{40001, 2}})}, migrated, plan));
    assert(plan.grants.size() == 2 && plan.grants[1].item_id == 40001 && plan.grants[1].count == 3);
    assert(plan.new_claims == (std::vector<int>{400, 401, 402}));
    for (int claim : plan.new_claims) migrated.claims.insert(claim);
    assert(plan_chapter_settlement(4, {slot(0, {{100001, 1}}), slot(1, {{40001, 1}}), slot(2, {{40001, 2}})}, migrated, plan));
    assert(plan.grants.empty() && plan.new_claims.empty());   // idempotent replay

    // Unknown first-win type stays owed; resolved empty slot is claimed without grants.
    assert(plan_chapter_settlement(5, {slot(0, {}, false), slot(1, {})}, migrated, plan));
    assert(plan.unresolved_slots == 1 && plan.grants.empty() && plan.new_claims == (std::vector<int>{501}));

    // Fresh account: no legacy assumptions, region 2 is granted by configuration.
    ChapterLedger fresh;
    add_legacy_chapter_claims({}, fresh);
    assert(plan_chapter_settlement(2, {slot(0, {{100001, 1}}), slot(1, {{40001, 2}})}, fresh, plan));
    assert(plan.grants.size() == 2 && plan.new_claims == (std::vector<int>{200, 201}));

    // Duplicate slot or overflow is refused.
    assert(!plan_chapter_settlement(6, {slot(1, {}), slot(1, {})}, fresh, plan));
    assert(!plan_chapter_settlement(6, {slot(1, {{40001, INT32_MAX}}), slot(2, {{40001, 1}})}, fresh, plan));
    std::puts("PASS: chapter reward tokens, ledger, legacy migration, idempotent settlement");
}
