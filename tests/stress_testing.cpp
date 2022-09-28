#include <iostream>
#include <numeric>
#include <random>

#include "../rb_tree.h"
#include "../avl_tree.h"
#include "../rb_tree_custom_invoke.h"
#include "../avl_tree_custom_invoke.h"

#include <gtest/gtest.h>

constexpr int mx_len = 200000;
constexpr int mx_iteration = 9;

TEST(StressTest, rb_tree)
{
    int iteration = mx_iteration;
    std::array<int, mx_len> s{};
    std::iota(s.begin(), s.end(), 0);
    while (iteration--)
    {
        auto seed = std::random_device()();
        std::cerr << "[          ] random seed = " << seed << std::endl;
        std::shuffle(s.begin(), s.end(), std::mt19937(seed));
        bbst::rb_tree<int, int, int, bbst::noop_metadata_updator_impl> rb;
        for (int i: s) rb.try_emplace(i);
        int i = 0;
        for (auto p: rb) EXPECT_EQ(p.key, i++);
        EXPECT_EQ(i, mx_len);
    }
}

TEST(StressTest, rb_tree_split_key)
{
    int iteration = mx_iteration;
    std::array<int, mx_len> s{};
    std::iota(s.begin(), s.end(), 0);
    std::array<int, mx_iteration> split_point{};
    std::uniform_int_distribution<int> distribution(0, mx_len - 1);
    auto seed = std::random_device()();
    auto outer = std::mt19937(seed);
    std::cerr << "[          ] random seed = " << seed << std::endl;
    for (int i = 1; i + 1 < mx_iteration; i++) split_point[i] = distribution(outer);
    split_point[0] = -1, split_point.back() = mx_len;
    while (iteration--)
    {
        auto split = split_point[iteration];
        std::shuffle(s.begin(), s.end(), outer);
        bbst::rb_tree<int, int, int, bbst::noop_metadata_updator_impl> rb;
        for (int i: s) rb.try_emplace(i);
        int i = 0;
        using rb_default_invoker = bbst::rb_tree_custom_invoke<int, int, int, bbst::noop_metadata_updator_impl, std::less<int>, bbst::rb_tree_custom_invoke_default_tag>;
        auto [l, r] = rb_default_invoker::split_by_key<false>(std::move(rb), split);
        for (auto p: l) EXPECT_EQ(p.key, i++);
        if (split >= 0) EXPECT_EQ(i, split);
        for (auto p: r) EXPECT_EQ(p.key, i++);
        EXPECT_EQ(i, mx_len);
    }
}

TEST(StressTest, rb_tree_order_statistic)
{
    int iteration = mx_iteration;
    std::array<int, mx_len> s{};
    std::iota(s.begin(), s.end(), 0);
    while (iteration--)
    {
        auto seed = std::random_device()();
        auto gen = std::mt19937(seed);
        std::cerr << "[          ] random seed = " << seed << std::endl;
        std::shuffle(s.begin(), s.end(), gen);
        bbst::rb_tree<int, int, int, bbst::order_statistic_metadata_updator_impl> rb;
        for (int i: s) rb.try_emplace(i);
        using rb_order_statistic_invoker = bbst::rb_tree_custom_invoke<int, int, int, bbst::order_statistic_metadata_updator_impl, std::less<int>, bbst::rb_tree_custom_invoke_order_statistic_tag>;
        EXPECT_EQ(rb_order_statistic_invoker::size(rb), mx_len);
        for (int i: s) EXPECT_EQ(rb_order_statistic_invoker::find_by_order(rb, i)->key, i);
        for (int i: s) EXPECT_EQ(rb_order_statistic_invoker::order_of_key(rb, i), i);
        EXPECT_EQ(rb_order_statistic_invoker::find_by_order(rb, mx_len), rb.end());

        //should work after split
        using rb_default_invoker = bbst::rb_tree_custom_invoke<int, int, int, bbst::order_statistic_metadata_updator_impl, std::less<int>, bbst::rb_tree_custom_invoke_default_tag>;
        auto split = std::uniform_int_distribution<int>(0, mx_len)(gen);
        auto [l, r] = rb_default_invoker::split_by_key<false>(std::move(rb), split);
        EXPECT_EQ(rb_order_statistic_invoker::size(l), split);
        EXPECT_EQ(rb_order_statistic_invoker::size(r), mx_len - split);
        for (int i = 0; i < split; i++) EXPECT_EQ(rb_order_statistic_invoker::find_by_order(l, i)->key, i);
        for (int i = 0; i < split; i++) EXPECT_EQ(rb_order_statistic_invoker::order_of_key(l, i), i);
        EXPECT_EQ(rb_order_statistic_invoker::find_by_order(l, split), l.end());
        for (int i = split; i < mx_len; i++) EXPECT_EQ(rb_order_statistic_invoker::find_by_order(r, i - split)->key, i);
        for (int i = split; i < mx_len; i++) EXPECT_EQ(rb_order_statistic_invoker::order_of_key(r, i), i - split);
        EXPECT_EQ(rb_order_statistic_invoker::find_by_order(r, mx_len - split), r.end());
    }
}

TEST(StressTest, avl_tree)
{
    int iteration = mx_iteration;
    std::array<int, mx_len> s{};
    std::iota(s.begin(), s.end(), 0);
    while (iteration--)
    {
        auto seed = std::random_device()();
        std::cerr << "[          ] random seed = " << seed << std::endl;
        std::shuffle(s.begin(), s.end(), std::mt19937(seed));
        bbst::avl_tree<int, int, int, bbst::noop_metadata_updator_impl> avl;
        for (int i: s) avl.try_emplace(i);
        int i = 0;
        for (auto p: avl) EXPECT_EQ(p.key, i++);
        EXPECT_EQ(i, mx_len);
    }
}

TEST(StressTest, avl_tree_split_key)
{
    int iteration = mx_iteration;
    std::array<int, mx_len> s{};
    std::iota(s.begin(), s.end(), 0);
    std::array<int, mx_iteration> split_point{};
    std::uniform_int_distribution<int> distribution(0, mx_len - 1);
    auto seed = std::random_device()();
    auto outer = std::mt19937(seed);
    std::cerr << "[          ] random seed = " << seed << std::endl;
    for (int i = 1; i + 1 < mx_iteration; i++) split_point[i] = distribution(outer);
    split_point[0] = -1, split_point.back() = mx_len;
    while (iteration--)
    {
        auto split = split_point[iteration];
        std::shuffle(s.begin(), s.end(), outer);
        bbst::avl_tree<int, int, int, bbst::noop_metadata_updator_impl> avl;
        for (int i: s) avl.try_emplace(i);
        int i = 0;
        using avl_default_invoker = bbst::avl_tree_custom_invoke<int, int, int, bbst::noop_metadata_updator_impl, std::less<int>, bbst::avl_tree_custom_invoke_default_tag>;
        auto [l, r] = avl_default_invoker::split_by_key<false>(std::move(avl), split);
        for (auto p: l) EXPECT_EQ(p.key, i++);
        if (split >= 0) EXPECT_EQ(i, split);
        for (auto p: r) EXPECT_EQ(p.key, i++);
        EXPECT_EQ(i, mx_len);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
