#include <iostream>
#include <numeric>
#include <bits/stdc++.h>

#include "rb_tree.h"
#include "avl_tree.h"
#include "rb_tree_custom_invoke.h"

int main()
{
    auto node_updator = []<class Ptr>(Ptr ptr)
    {
        auto get = [](Ptr ptr)
        {
            return ptr == nullptr ? 0 : ptr->metadata();
        };
        if (ptr == nullptr) return;
        ptr->metadata() = get(ptr->left) + get(ptr->right) + ptr->key();
    };

    std::array<int, 9> s{};
    std::iota(s.begin(), s.end(), 0);
    std::random_shuffle(s.begin(), s.end());
    BBST::rb_tree<int, int, int, BBST::order_statistic_metadata_updator> bst;
    for (int i: s) bst.try_emplace(i);
    for (auto p: bst) std::cout << p.key << ' ';
    std::cout << '\n';
    using order_statistic_invoker = BBST::rb_tree_custom_invoke<int, int, int, BBST::order_statistic_metadata_updator, std::less<int>, BBST::rb_tree_custom_invoke_order_statistic_tag>;
    std::cout << order_statistic_invoker::find_by_order(bst, 3)->key << '\n';
    using default_invoker = BBST::rb_tree_custom_invoke<int, int, int, BBST::order_statistic_metadata_updator, std::less<int>, BBST::rb_tree_custom_invoke_default_tag>;
    auto [l, r] = default_invoker::split_by_key<true>(std::move(bst), 7);
    for (auto p: l) std::cout << p.key << ' ';
    std::cout << std::endl;
    for (auto p: r) std::cout << p.key << ' ';
    std::cout << std::endl;
}
