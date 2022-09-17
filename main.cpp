#include <iostream>
#include <numeric>
#include <bits/stdc++.h>

#include "rb_tree.h"
#include "avl_tree.h"


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

    std::array<int,9> s{};
    std::iota(s.begin(),s.end(),0);
    do {
        BBST::avl_tree<int, int, int, decltype(node_updator)> bst;
        for(int i:s) bst.try_emplace(i);
        for(auto p:bst) std::cout << p.key << ' ';
        std::cout << '\n';
    } while(std::next_permutation(s.begin(),s.end()));
}
