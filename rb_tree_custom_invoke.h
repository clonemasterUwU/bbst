#ifndef BBST_RB_TREE_CUSTOM_INVOKE_H
#define BBST_RB_TREE_CUSTOM_INVOKE_H

#include <type_traits>
#include "rb_tree.h"
#include "tree_custom_invoke.h"

namespace bbst
{

    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t, class tag>
    struct rb_tree_custom_invoke {};

    struct rb_tree_custom_invoke_default_tag {};

    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t>
    struct rb_tree_custom_invoke<key_t, mapped_t, metadata_t, metadata_updator_t, comparator_t, rb_tree_custom_invoke_default_tag>
    {
        using rb_tree_t = rb_tree<key_t, mapped_t, metadata_t, metadata_updator_t, comparator_t>;
        using rb_tree_header_t = rb_tree_header<key_t, mapped_t, metadata_t>;
        using rb_tree_node_ptr_t = typename rb_tree_t::rb_tree_node_ptr_t;

        static inline rb_tree_header_t to_rb_tree_header(rb_tree_t &&tree)
        {
            rb_tree_node_ptr_t root = std::exchange(tree.end_node_.left, nullptr);
            tree.begin_node_ = &tree.end_node_;
            uint32_t black_height = std::exchange(tree.black_height_, 1);
            return {root, black_height};
        };

        template<bool equal_on_left_side>
        static std::pair<rb_tree_t, rb_tree_t> split_by_key(rb_tree_t &&tree, const key_t &key)
        {

            auto &comparator = tree.comp_;
            auto &metadata_updator = tree.updator_;

            auto split_routine = [&comparator, &metadata_updator](auto self, rb_tree_header_t header
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
                    auto [left_header, right_header] = self(self, rb_tree_header_t(header.root_->left, header.black_height_ - left_is_black), key);
                    bool right_is_black = header.root_->right == nullptr || std::exchange(header.root_->right->is_black_, true);
                    return {left_header, bbst::rb_tree_join_x(right_header, header.root_, rb_tree_header_t(header.root_->right,
                            header.black_height_ - right_is_black), metadata_updator, comparator)};
                }
                else
                {
                    if (comparator(header.root_->key(), key))
                    {
                        //header.root_ < key
                        bool right_is_black = header.root_->right == nullptr || std::exchange(header.root_->right->is_black_, true);
                        auto [left_header, right_header] = self(self, rb_tree_header_t(header.root_->right, header.black_height_ - right_is_black), key);
                        bool left_is_black = header.root_->left == nullptr || std::exchange(header.root_->left->is_black_, true);
                        return {bbst::rb_tree_join_x(rb_tree_header_t(header.root_->left, header.black_height_ - left_is_black), header
                                .root_, left_header, metadata_updator, comparator), right_header};
                    }
                    else
                    {
                        //==
                        bool left_is_black = header.root_->left == nullptr || std::exchange(header.root_->left->is_black_, true);
                        bool right_is_black = header.root_->right == nullptr || std::exchange(header.root_->right->is_black_, true);
                        rb_tree_node_ptr_t left = header.root_->left;
                        rb_tree_node_ptr_t right = header.root_->right;
                        if constexpr(equal_on_left_side)
                        {
                            return {bbst::rb_tree_join_x(rb_tree_header_t(left, header.black_height_ - left_is_black), header
                                    .root_, rb_tree_header_t::empty_header(), metadata_updator, comparator),
                                    rb_tree_header_t(right, header.black_height_ - right_is_black)};
                        }
                        else
                        {
                            return {rb_tree_header_t(left, header.black_height_ - left_is_black),
                                    bbst::rb_tree_join_x(rb_tree_header_t::empty_header(), header.root_, rb_tree_header_t(right,
                                            header.black_height_ - right_is_black), metadata_updator, comparator)};
                        }
                    }
                }
            };
            auto [l, r] = split_routine(split_routine, to_rb_tree_header(std::move(tree)), key);
            ASSERT(rb_tree_header_invariant(l), "post condition failed");
            ASSERT(rb_tree_header_invariant(r), "post condition failed");
            return {rb_tree_t(l, metadata_updator, comparator), rb_tree_t(r, metadata_updator, comparator)};
        }
    };

    struct rb_tree_custom_invoke_order_statistic_tag {};

    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t>
    requires (std::is_integral_v<metadata_t> &&
              bbst::is_order_statistic_metadata_updator<metadata_updator_t, bbst::rb_tree_node<bbst::exposure<key_t, mapped_t, metadata_t>> *>)
    struct rb_tree_custom_invoke<key_t, mapped_t, metadata_t, metadata_updator_t, comparator_t, rb_tree_custom_invoke_order_statistic_tag>
    {
        using rb_tree_t = rb_tree<key_t, mapped_t, metadata_t, metadata_updator_t, comparator_t>;
        using rb_tree_node_ptr_t = typename rb_tree_t::rb_tree_node_ptr_t;
        using iterator = typename rb_tree_t::iterator;
        using const_iterator = typename rb_tree_t::const_iterator;

        static const_iterator find_by_order(const rb_tree_t &tree, size_t index)
        {
            rb_tree_node_ptr_t node = tree.end_node_.left;
            if (node == nullptr || metadata_updator_t::get_order_metadata(node) <= index)
                return tree.end();
            while (true)
            {
                auto left_count = metadata_updator_t::get_order_metadata(node->left);
                if (left_count == index)
                    return const_iterator(node);
                else if (left_count > index)
                    node = node->left;
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
            if (node == nullptr || metadata_updator_t::get_order_metadata(node) <= index)
                return tree.end();
            while (true)
            {
                auto left_count = metadata_updator_t::get_order_metadata(node->left);
                if (left_count == index)
                    return iterator(node);
                else if (left_count > index)
                    node = node->left;
                else
                {
                    node = node->right;
                    index -= left_count + 1;//minus node
                }
            }
            //std::unreachable();//C++23 macro
            ASSERT(false, "unreachable");
        }

        static size_t size(const rb_tree_t &tree)
        {
            return metadata_updator_t::get_order_metadata(tree.end_node_.left);
        }

        static size_t order_of_key(const rb_tree_t &tree, const key_t &key)
        {
            ASSERT(false, "unimplemented");
        }
    };
}

namespace bbst
{
    template<class T>
    struct is_pair_integral : std::false_type {};

    template<class T>
    requires(std::is_integral_v<T>)
    struct is_pair_integral<std::pair<T, T>> : std::true_type {};

    template<class T> inline constexpr bool is_pair_integral_v = is_pair_integral<T>::value;

    struct interval_metadata_updator
    {
        template<class rb_tree_node_ptr_t>
        static inline typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type get(rb_tree_node_ptr_t ptr)
        {
            using T = typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type::first_type;
            if (ptr == nullptr)
            {
                return {std::numeric_limits<T>::max(), std::numeric_limits<T>::min()};
            }
            return ptr->metadata();
        }

        template<class rb_tree_node_ptr_t>
        void operator()(rb_tree_node_ptr_t ptr) const
        requires (is_pair_integral_v < typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type > &&
                  is_pair_integral_v < typename std::remove_pointer_t<rb_tree_node_ptr_t>::key_type > &&
                  std::is_same_v<typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type::first_type, typename std::remove_pointer_t<rb_tree_node_ptr_t>::key_type::first_type>)
        {
            auto left = interval_metadata_updator::get(ptr->left);
            auto right = interval_metadata_updator::get(ptr->right);
            ptr->metadata() = std::min(left.first, ptr->key().first), std::max(right.second, ptr->key().second);
        }
    };

    struct rb_tree_custom_invoke_interval_tag
    {
    };

    struct sum_metadata_updator
    {
        template<class rb_tree_node_ptr_t>
        requires(std::is_arithmetic_v<typename std::remove_pointer_t<rb_tree_node_ptr_t>::metadata_type>)
        void operator()(rb_tree_node_ptr_t ptr) const
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
