# BBST
An STL-like, header-only collection of balanced binary search trees that support ```O(log n)``` split/join oeprators (and more).

# Build from Source

Requirements:

* CMake 3.15+
* any C++ 20 compatible compiler (yes, concepts)

At the cloned directory:

```sh
cmake -B build -H.
#build static library
cmake --build build --target bbst
#optional, build test
cmake --build build --target stress_testing exhaustive_testing
#optional, build benchmark
cmake --build build --target benchmarkme
```


# Simple Usecase
```cpp
#include <iostream>
#include <utility>
#include "rb_tree.h"
#include "rb_tree_custom_invoke.h"
int main()
{
    bbst::rb_tree<int, int, int, bbst::order_statistic_metadata_updator_impl> rb;
    std::array<int,7> s {0,1,5,3,10,7,2};
    for (int i: s) 
        rb.try_emplace(i,0);
    using rb_order_statistic_invoker = bbst::rb_tree_custom_invoke<int, int, int, bbst::order_statistic_metadata_updator_impl, std::less<int>, bbst::rb_tree_custom_invoke_order_statistic_tag>;
    std::cout << rb_order_statistic_invoker::find_by_order(rb,4) << std::endl; //5
    std::cout << rb_order_statistic_invoker::order_of_key(rb,9) << std::endl; //6
    using rb_default_invoker = bbst::rb_tree_custom_invoke<int, int, int, bbst::order_statistic_metadata_updator_impl, std::less<int>, bbst::rb_tree_custom_invoke_default_tag>;
    auto [left, right] = rb_default_invoker::split_by_key<true>(std::move(rb),3);
    for(auto p:left)
        std::cout << p->key << ' ';
    std::cout << std::endl; // 0 1 2 3
    for(auto p:right)
        std::cout << p->key << ' ';
    std::cout << std::endl // 5 7 10
}
```
# Customize your own tree

# License
BBST is released under the [MIT license](LICENSE)

