#include "../rb_tree.h"
#include "../avl_tree.h"
#include "../rb_tree_custom_invoke.h"
#include "../avl_tree_custom_invoke.h"

#include <gtest/gtest.h>
#include <numeric>
#include <array>

TEST(ExhaustiveTest, rb_tree)
{
    constexpr int mx = 9;
    std::array<int, mx> s{};
    std::iota(s.begin(), s.end(), 0);
    do
    {
        bbst::rb_tree<int, int, int, bbst::noop_metadata_updator_impl> rb;
        for (int i: s) rb.try_emplace(i);
        int i = 0;
        for (auto p: rb) EXPECT_EQ(p.key, i++);
        EXPECT_EQ(i, mx);
    } while (std::next_permutation(s.begin(), s.end()));
}

TEST(ExhaustiveTest, avl_tree)
{
    constexpr int mx = 9;
    std::array<int, mx> s{};
    std::iota(s.begin(), s.end(), 0);
    do
    {
        bbst::avl_tree<int, int, int, bbst::noop_metadata_updator_impl> avl;
        for (int i: s) avl.try_emplace(i);
        int i = 0;
        for (auto p: avl) EXPECT_EQ(p.key, i++);
        EXPECT_EQ(i, mx);
    } while (std::next_permutation(s.begin(), s.end()));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
