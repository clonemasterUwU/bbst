#include <iostream>
#include <numeric>

#include "rb_tree.h"

namespace BBST
{

}

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

    BBST::RedBlackTree<int, int, int, decltype(node_updator)> bst;
    bst[0], bst[1], bst[2];
}
