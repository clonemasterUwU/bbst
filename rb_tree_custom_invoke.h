#ifndef BBST_RB_TREE_CUSTOM_INVOKE_H
#define BBST_RB_TREE_CUSTOM_INVOKE_H

#include "rb_tree.h"

namespace BBST
{
    struct rb_tree_custom_invoke_default_tag
    {
    };

    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t, class tag>
    struct rb_tree_custom_invoke
    {

        using rb_tree_t = rb_tree<key_t, mapped_t, metadata_t, metadata_updator_t, comparator_t>;
        using rb_tree_header_t = rb_tree_header<key_t, mapped_t, metadata_t>;
        using rb_tree_node_ptr_t = typename rb_tree_t::rb_tree_node_ptr_t;

        static inline rb_tree_header_t to_rb_tree_header(rb_tree_t &&tree)
        {
            if (tree.empty()) return rb_tree_header_t::empty_header();
            rb_tree_node_ptr_t root = std::exchange(tree.end_node_.left, nullptr);
            tree.begin_node_ = &tree.end_node_;
            uint32_t black_height = std::exchange(tree.black_height_, 1);
            ASSERT(rb_tree_header_invariant(rb_tree_header{root, black_height}), "the tree is ill formed");
            return {root, black_height};
        }

        template<bool equal_on_left_side>
        static std::pair<rb_tree_t, rb_tree_t> split_by_key(rb_tree_t &&tree, const key_t &key)
        {

            auto &comparator = tree.comp_;
            auto &metadata_updator = tree.updator_;
            auto print = [](rb_tree_header_t header){
                auto recursive = [](auto self,rb_tree_node_ptr_t node)->void {
                    if(node == nullptr) return;
                    self(self,node->left);
                    std::cout << node->key() << ' ';
                    self(self,node->right);
                };
                recursive(recursive,header.root_);
                std::cout << std::endl;
            };
            auto split_routine = [&comparator, &metadata_updator,&print](auto self, rb_tree_header_t header
                                                                  , const key_t &key) -> std::pair<rb_tree_header_t, rb_tree_header_t>
            {
                if (header.empty())
                {
                    return {rb_tree_header_t::empty_header(), rb_tree_header_t::empty_header()};
                }
                if (comparator(key, header.root_->key()))
                {
                    //key < header.root_
                    bool left_is_black = header.root_->left == nullptr || std::exchange(header.root_->left->is_black_, true);
                    auto [left_header, right_header] = self(self, rb_tree_header_t(header.root_->left, header.black_height_ - left_is_black),key);
                    print(left_header);
                    print(right_header);
                    bool right_is_black = header.root_->right == nullptr || std::exchange(header.root_->right->is_black_, true);
                    return {left_header, BBST::rb_tree_join_x(right_header, header.root_,
                                                              rb_tree_header_t(header.root_->right, header.black_height_ - right_is_black),
                                                              metadata_updator, comparator)};
                }
                else
                {
                    if (comparator(header.root_->key(), key))
                    {
                        //header.root_ < key
                        bool right_is_black = header.root_->right == nullptr || std::exchange(header.root_->right->is_black_, true);

                        auto [left_header, right_header] = self(self, rb_tree_header_t(header.root_->right, header.black_height_ - right_is_black),key);
                        print(left_header);
                        print(right_header);
                        bool left_is_black = header.root_->left == nullptr || std::exchange(header.root_->left->is_black_, true);
                        return {BBST::rb_tree_join_x(rb_tree_header_t(header.root_->left, header.black_height_ - left_is_black), header.root_,
                                                     left_header, metadata_updator, comparator), right_header};
                    }
                    else
                    {
                        //==
                        bool left_is_black = header.root_->left == nullptr || std::exchange(header.root_->left->is_black_, true);
                        bool right_is_black = header.root_->right == nullptr || std::exchange(header.root_->right->is_black_, true);
                        rb_tree_node_ptr_t left = header.root_->left;
                        rb_tree_node_ptr_t right =header.root_->right;
                        if constexpr(equal_on_left_side)
                        {
                            return {BBST::rb_tree_join_x(rb_tree_header_t(left, header.black_height_ - left_is_black), header.root_,
                                                         rb_tree_header_t::empty_header(), metadata_updator, comparator),
                                    rb_tree_header_t(right, header.black_height_ - right_is_black)};
                        }
                        else
                        {
                            return {rb_tree_header_t(left, header.black_height_ - left_is_black),
                                    BBST::rb_tree_join_x(rb_tree_header_t::empty_header(), header.root_,
                                                         rb_tree_header_t(right, header.black_height_ - right_is_black),
                                                         metadata_updator, comparator)};
                        }
                    }
                }
            };
            auto [l, r] = split_routine(split_routine, to_rb_tree_header(std::move(tree)), key);
            ASSERT(rb_tree_header_invariant(l),"post condition failed");
            ASSERT(rb_tree_header_invariant(r),"post condition failed");
//            print(l);
//            print(r);
            return {rb_tree_t(l, comparator, metadata_updator), rb_tree_t(r, comparator, metadata_updator)};
        }
    };

    struct order_statistic_metadata_updator
    {
        template<class rb_tree_node_ptr_t>
        void operator()(rb_tree_node_ptr_t ptr) const
        requires(std::is_integral_v<typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type>)
        {

            ptr->metadata() = 1 + order_statistic_metadata_updator::get(ptr->left) + order_statistic_metadata_updator::get(ptr->right);
        }

        template<class rb_tree_node_ptr_t>
        requires(std::is_integral_v<typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type>)
        static inline typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type get(rb_tree_node_ptr_t p)
        {
            return p == nullptr ? 0 : p->metadata();
        };
    };

    struct rb_tree_custom_invoke_order_statistic_tag
    {
    };

    template<class key_t, class mapped_t, class metadata_t, class comparator_t>
    requires(std::is_integral_v<metadata_t>)
    struct rb_tree_custom_invoke<key_t, mapped_t, metadata_t, order_statistic_metadata_updator, comparator_t, rb_tree_custom_invoke_order_statistic_tag>
    {
        using rb_tree_t = rb_tree<key_t, mapped_t, metadata_t, order_statistic_metadata_updator, comparator_t>;

        using rb_tree_node_ptr_t = typename rb_tree_t::rb_tree_node_ptr_t;
        using iterator = typename rb_tree_t::iterator;
        using const_iterator = typename rb_tree_t::const_iterator;

        static const_iterator find_by_order(const rb_tree_t &tree, size_t index)
        {
            rb_tree_node_ptr_t node = tree.end_node_.left;
            if (node == nullptr || node->metadata() <= index)
                return tree.end();
            while (true)
            {
                auto left_count = order_statistic_metadata_updator::get(node->left);
                if (left_count == index)
                {
                    return const_iterator(node);
                }
                else if (left_count > index)
                {
                    node = node->left;
                }
                else
                {
                    node = node->right;
                    index -= left_count + 1;//minus node
                }
            }
            ASSERT(false, "unreachable");
        }

        static iterator find_by_order(rb_tree_t &tree, size_t index)
        {
            rb_tree_node_ptr_t node = tree.end_node_.left;
            if (node == nullptr || node->metadata() <= index)
                return tree.end();
            while (true)
            {
                auto left_count = order_statistic_metadata_updator::get(node->left);
                if (left_count == index)
                {
                    return iterator(node);
                }
                else if (left_count > index)
                {
                    node = node->left;
                }
                else
                {
                    node = node->right;
                    index -= left_count + 1;//minus node
                }
            }
            ASSERT(false, "unreachable");
        }

        static size_t size(const rb_tree_t &tree)
        {
            return order_statistic_metadata_updator::get(tree.end_node_.left);
        }

        static size_t order_by_key(const rb_tree_t &tree, const key_t &key)
        {
            ASSERT(false, "unimplemented");
        }
    };

    struct sum_metadata_updator
    {
        template<class rb_tree_node_ptr_t>
        void operator()(rb_tree_node_ptr_t ptr)
        requires(std::is_arithmetic_v<typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type>)
        {
            auto get = [](rb_tree_node_ptr_t p)
            {
                return p == nullptr ? 0 : p->metadata();
            };
            ptr->metadata() = ptr->key() + get(ptr->left) + get(ptr->right);
        }
    };
}
#endif //BBST_RB_TREE_CUSTOM_INVOKE_H
