#include <iostream>
#include <numeric>
#include <random>
#include <bits/stdc++.h>

#include "rb_tree.h"
#include "avl_tree.h"
#include "rb_tree_custom_invoke.h"
#include "avl_tree_custom_invoke.h"

int main()
{
    std::array<int, 1000> s{};
    std::iota(s.begin(), s.end(), 0);
    std::shuffle(s.begin(), s.end(), std::mt19937(0));
    bbst::rb_tree<int, int, int, bbst::order_statistic_metadata_updator_impl> rb;
    bbst::avl_tree<int, int, int, bbst::order_statistic_metadata_updator_impl> avl;

    for (int i: s) rb.try_emplace(i),avl.try_emplace(i);
    for (auto p: rb) std::cout << p.key << ' ';
    std::cout << '\n';
    for (auto p: avl) std::cout << p.key << ' ';
    std::cout << '\n';

    using rb_order_statistic_invoker = bbst::rb_tree_custom_invoke<int, int, int, bbst::order_statistic_metadata_updator_impl, std::less<int>, bbst::rb_tree_custom_invoke_order_statistic_tag>;
    std::cout << rb_order_statistic_invoker::find_by_order(rb, 723)->key << '\n';
    using rb_default_invoker = bbst::rb_tree_custom_invoke<int, int, int, bbst::order_statistic_metadata_updator_impl, std::less<int>, bbst::rb_tree_custom_invoke_default_tag>;
    using avl_default_invoker = bbst::avl_tree_custom_invoke<int, int, int, bbst::order_statistic_metadata_updator_impl, std::less<int>, bbst::avl_tree_custom_invoke_default_tag>;
    auto [rbl, rbr] = rb_default_invoker::split_by_key<true>(std::move(rb), 126);
    for (auto p: rbl) std::cout << p.key << ' ';
    std::cout << std::endl;
    for (auto p: rbr) std::cout << p.key << ' ';
    std::cout << std::endl;
    auto [avll, avlr] = avl_default_invoker::split_by_key<true>(std::move(avl), 126);
    for (auto p: avll) std::cout << p.key << ' ';
    std::cout << std::endl;
    for (auto p: avlr) std::cout << p.key << ' ';
    std::cout << std::endl;
}
