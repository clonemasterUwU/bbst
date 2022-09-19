#ifndef BBST_AVL_TREE_H
#define BBST_AVL_TREE_H

#include <concepts>
#include <memory>
#include "tree_utils.h"

//invariant debug
namespace BBST
{

    template<class avl_tree_node_ptr_t>
    uint32_t avl_tree_invariant(avl_tree_node_ptr_t ptr)
    {
        if (ptr == nullptr)
            return 1;
        if (ptr->left != nullptr && ptr->left->parent != ptr)
            return 0;

        if (ptr->right != nullptr && ptr->right->parent != ptr)
            return 0;

        if (ptr->left != nullptr && ptr->right != nullptr && ptr->left == ptr->right)
            return 0;

        auto left_height = avl_tree_invariant(ptr->left);
        if (left_height == 0)
            return 0;

        auto right_height = avl_tree_invariant(ptr->right);
        if (right_height == 0)
            return 0;

        if (right_height != left_height + ptr->height_diff_) //this won't overflow
            return 0;

        return 1 + std::max(left_height, right_height);
    }

    template<class avl_tree_header_t>
    bool avl_tree_header_invariant(avl_tree_header_t header)
    {
        auto height = avl_tree_invariant(header.root_);
        if (height == 0)
            return false;
        if (height != header.height_)
            return false;
        if (header.root_ != nullptr)
        {
            if (tree_min(header.root_) != header.begin_)
                return false;
        }
        else
        {
            if (header.begin_ != nullptr)
                return false;
        }
        return true;
    }
}

namespace BBST
{
    template<class exposure_t>
    struct avl_tree_node : public base_tree_node<avl_tree_node<exposure_t>>
    {
        //expose type info
        typedef typename tree_node_base_traits<avl_tree_node>::value_type value_type;
        typedef typename tree_node_base_traits<avl_tree_node>::key_type key_type;
        typedef typename tree_node_base_traits<avl_tree_node>::metadata_type metadata_type;
        int32_t height_diff_
        //                : 2
        ;
        value_type value_;

        template<class... Args>
        explicit avl_tree_node(int32_t height_diff, Args... args)
                :
                height_diff_(height_diff)
                , value_(std::forward<Args>(args)...)
        {}

        ~avl_tree_node()
        {
            delete this->left;
            delete this->right;
        }

        inline value_type &value() noexcept
        {
            return value_;
        }

        inline const value_type &value() const noexcept
        {
            return value_;
        }

        inline key_type &key() const noexcept
        {
            return value_.key;
        }

        inline metadata_type &metadata() noexcept
        {
            return value_.metadata;
        }

        inline const metadata_type &metadata() const noexcept
        {
            return value_.metadata;
        }
    };

    template<class exposure_t>
    struct tree_node_base_traits<avl_tree_node<exposure_t>>
    {
        typedef exposure_t value_type;
        typedef avl_tree_node<exposure_t> impl_type;
        typedef typename exposure_t::key_type key_type;
        typedef typename exposure_t::mapped_type mapped_type;
        typedef typename exposure_t::metadata_type metadata_type;
    };

    template<class key_t, class mapped_t, class metadata_t>
    struct avl_tree_header
    {
    private:
        typedef avl_tree_node<BBST::exposure<key_t, mapped_t, metadata_t>> avl_tree_node_t;
        typedef avl_tree_node_t *avl_tree_node_ptr_t;
        typedef base_tree_node<avl_tree_node_t> base_tree_node_t;
    public:
        typedef tree_forward_iterator_<base_tree_node_t> iterator;
        typedef tree_forward_const_iterator_<base_tree_node_t> const_iterator;

        avl_tree_node_ptr_t root_;
        avl_tree_node_ptr_t begin_;
        uint32_t height_;

        typedef avl_tree_header<key_t, mapped_t, metadata_t> avl_tree_header_t;

        //        template<class metadata_updator_t>
        avl_tree_header(avl_tree_node_ptr_t root, avl_tree_node_ptr_t begin, uint32_t height)
                :
                root_(root)
                , begin_(begin)
                , height_(height)
        {}

        inline iterator begin() noexcept
        {
            return iterator(begin_);
        }

        inline const_iterator begin() const noexcept
        {
            return const_iterator(begin_);
        };

        inline iterator end() noexcept
        {
            return iterator(root_->parent);
        }

        inline const_iterator end() const noexcept
        {
            return const_iterator(root_->parent);
        }
    };

    template<class avl_tree_node_ptr_t, class metadata_updator_t>
    std::pair<bool, avl_tree_node_ptr_t> avl_tree_insert_fixup(avl_tree_node_ptr_t Z, avl_tree_node_ptr_t root
                                                               , const metadata_updator_t &updator) noexcept(std::is_nothrow_invocable_v<const metadata_updator_t &, avl_tree_node_ptr_t>)
    {
        ASSERT(Z == root || abs_difference(avl_tree_invariant(Z->parent->left), avl_tree_invariant(Z->parent->right)) <= 2, "pre condition failed");
        bool height_inc = true;
        while (Z != root)
        {
            avl_tree_node_ptr_t X = Z->parent_unsafe();
            if (X->right == Z)
            {
                if (X->height_diff_ > 0)
                {
                    ASSERT(X->height_diff_ == 1, "X is right heavy");
                    if (Z->height_diff_ < 0)
                    {
                        avl_tree_node_ptr_t Y = (X == root) ? tree_root_right_left_rotate(X, Z) : unguarded_tree_right_left_rotate(X, Z);
                        if (Y->height_diff_ == 0)
                        {
                            X->height_diff_ = Z->height_diff_ = 0;
                        }
                        else
                        {
                            if (Y->height_diff_ > 0)
                                X->height_diff_ = -1, Z->height_diff_ = 0;
                            else
                                X->height_diff_ = 0, Z->height_diff_ = 1;
                            Y->height_diff_ = 0;
                        }
                        updator(X);
                        updator(Z);
                        updator(Y);
                        if (X == root)
                        {
                            ASSERT(avl_tree_invariant(Y), "post condition failed");
                            return {false, Y};
                        }
                        Z = Y;
                        height_inc = false;
                        break;
                    }
                    else
                    {
                        if (X == root)
                            tree_root_left_rotate(X);
                        else
                            unguarded_tree_left_rotate(X);
                        X->height_diff_ = Z->height_diff_ = 0;
                        updator(X);
                        updator(Z);
                        if (X == root)
                        {
                            ASSERT(avl_tree_invariant(Z), "post condition failed");
                            return {false, Z};
                        }
                        else
                        {
                            Z = X;
                            height_inc = false;
                            break;
                        }
                    }
                }
                else
                {
                    Z = X;
                    updator(X);
                    if (X->height_diff_ < 0)
                    {
                        X->height_diff_ = 0;
                        height_inc = false;
                        break;
                    }
                    X->height_diff_ = 1;
                }
            }
            else
            {
                if (X->height_diff_ < 0)
                {
                    ASSERT(X->height_diff_ == -1, "X is left heavy");
                    if (Z->height_diff_ > 0)
                    {
                        avl_tree_node_ptr_t Y = (X == root) ? tree_root_left_right_rotate(X, Z) : unguarded_tree_left_right_rotate(X, Z);
                        if (Y->height_diff_ == 0)
                        {
                            X->height_diff_ = Z->height_diff_ = 0;
                        }
                        else
                        {
                            if (Y->height_diff_ < 0)
                                X->height_diff_ = 1, Z->height_diff_ = 0;
                            else
                                X->height_diff_ = 0, Z->height_diff_ = -1;
                            Y->height_diff_ = 0;
                        }
                        updator(X);
                        updator(Z);
                        updator(Y);
                        if (X == root)
                        {
                            ASSERT(avl_tree_invariant(Y), "post condition failed");
                            return {false, Y};
                        }
                        Z = Y;
                        height_inc = false;
                        break;
                    }
                    else
                    {
                        if (X == root)
                            tree_root_right_rotate(X);
                        else
                            unguarded_tree_right_rotate(X);
                        X->height_diff_ = Z->height_diff_ = 0;
                        updator(X);
                        updator(Z);
                        if (X == root)
                        {
                            ASSERT(avl_tree_invariant(Z), "post condition failed");

                            return {false, Z};
                        }
                        else
                        {
                            Z = X;
                            height_inc = false;
                            break;
                        }
                    }
                }
                else
                {
                    Z = X;
                    updator(X);
                    if (X->height_diff_ > 0)
                    {
                        X->height_diff_ = 0;
                        break;
                    }
                    X->height_diff_ = -1;
                }
            }
        }
        ASSERT(avl_tree_invariant(Z), "post condition failed");
        while (Z != root)
        {
            Z = Z->parent_unsafe();
            updator(Z);
        }
        return {height_inc, Z};
    }

    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t=std::less<key_t>>
    requires (std::predicate<const comparator_t &, const key_t &, const key_t &> &&
              std::regular_invocable<const metadata_updator_t &, rb_tree_node<BBST::exposure<key_t, mapped_t, metadata_t>> *>)
    class avl_tree
    {
    private:
        typedef avl_tree_node<exposure<key_t, mapped_t, metadata_t>> avl_tree_node_t;
        typedef base_tree_node<avl_tree_node_t> base_tree_node_t;
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef avl_tree_node_t *avl_tree_node_ptr_t;
        typedef avl_tree_header<key_t, mapped_t, metadata_t> avl_tree_header_t;
        typedef typename avl_tree_node_t::value_type value_type;
    public:
        typedef tree_bidirectional_iterator_<base_tree_node_t> iterator;
        typedef tree_bidirectional_const_iterator_<base_tree_node_t> const_iterator;

    private:
        base_tree_node_t end_node_;
        base_tree_node_ptr_t begin_node_;
        comparator_t comp_;
        metadata_updator_t updator_;
        uint32_t height_;

        template<class... Args>
        avl_tree_node_ptr_t construct_node(Args... args)
        {
            return new avl_tree_node_t(std::forward<Args>(args)...);
        }

        std::pair<avl_tree_node_ptr_t &, base_tree_node_ptr_t> inline find_equal_or_insert_pos(const key_t &key)
        {
            return BBST::find_equal_or_insert_pos<key_t, base_tree_node_ptr_t, avl_tree_node_ptr_t, comparator_t>(key, &end_node_, comp_);
        }

        void insert_node_at(base_tree_node_ptr_t parent, avl_tree_node_ptr_t &child, avl_tree_node_ptr_t new_node) noexcept
        {
            new_node->left = nullptr;
            new_node->right = nullptr;
            new_node->parent = parent;
            child = new_node;
            if (begin_node_->left != nullptr)
                begin_node_ = begin_node_->left;
            auto [height_inc, new_root] = avl_tree_insert_fixup(new_node, end_node_.left, updator_);
            end_node_.left = new_root;
            height_ += height_inc;
        }

        template<class... Args>
        std::pair<iterator, bool> emplace_key_args(key_t key, Args... args)
        {

            auto [child, parent] = find_equal_or_insert_pos(key);
            if (child == nullptr)
            {
                avl_tree_node_ptr_t new_node = construct_node(0, std::forward<key_t>(key), std::forward<Args>(args)...);
                insert_node_at(parent, child, new_node);
                return {iterator(new_node), true};
            }
            return {iterator(child), false};
        }

    public:

        template<class comparator_forward_t=comparator_t, class metadata_updator_forward_t=metadata_updator_t>
        requires (std::is_same_v<comparator_t, std::remove_reference_t<comparator_forward_t>> &&
                  std::is_same_v<metadata_updator_t, std::remove_reference_t<metadata_updator_forward_t>>)
        avl_tree(metadata_updator_forward_t &&updator = metadata_updator_t(), comparator_forward_t &&comp = comparator_t())
                :
                end_node_(nullptr, nullptr, nullptr)
                , begin_node_(&end_node_)
                , updator_(std::forward<metadata_updator_forward_t>(updator))
                , comp_(std::forward<comparator_forward_t>(comp))
                , height_(1)
        {

        }

        template<class... Args>
        inline std::pair<iterator, bool> try_emplace(key_t key, Args...args)
        {
            return emplace_key_args(std::forward<key_t>(key), std::forward<Args>(args)...);
        }

        ~avl_tree()
        {
            delete end_node_.left;
        }
        //        avl_tree_header_t avl_tree_join_x(avl_tree_header_t left, avl_tree_node_ptr_t x, avl_tree_header_t right
        //                                          , const metadata_updator_t &updator
        //                                          , const comparator_t &comparator) noexcept(std::is_nothrow_invocable_v<const metadata_updator_t &, avl_tree_node_ptr_t>)
        //        {
        //            ASSERT(avl_tree_header_invariant(left),"left header invariant false");
        //            ASSERT(left.root_ == nullptr || comparator(BBST::tree_max(left.root_)->key(), x->key()),
        //                   "left tree must be less than x");
        //            ASSERT(avl_tree_header_invariant(right), "right header invariant false");
        //            ASSERT(right.root_ == nullptr || comparator(x->key(), BBST::tree_min(right.root_)->key()),
        //                   "right tree must be greater than x");
        //            if (left.height_ == right.height_)
        //            {
        //                if (left.root_)
        //                {
        //                    x->left = left.root_;
        //                    left.root_->parent = x;
        //                }
        //                if (right.root_)
        //                {
        //                    x->right = right.root_;
        //                    right.root_->parent = x;
        //                }
        //                x->height_diff_ = 0;
        //                left.root_ = x;
        //                left.height_++;
        //                return left;
        //            } else if(){
        //
        //            }
        //        }




        inline iterator begin() noexcept
        {
            return iterator(begin_node_);
        }

        [[nodiscard]] inline const_iterator begin() const noexcept
        {
            return const_iterator(begin_node_);
        }

        inline iterator end() noexcept
        {
            return iterator(&end_node_);
        }

        [[nodiscard]] inline const_iterator end() const noexcept
        {
            return const_iterator(&end_node_);
        }

        inline comparator_t &value_comp() noexcept
        {
            return comp_;
        }

        [[nodiscard]] inline const comparator_t &value_comp() const noexcept
        {
            return comp_;
        }

        iterator lower_bound(const key_t &key)
        {
            return iterator(BBST::lower_bound(&end_node_, key, comp_));
        }

        [[nodiscard]] const_iterator lower_bound(const key_t &key) const
        {
            return const_iterator(BBST::lower_bound(&end_node_, key, comp_));
        }

        iterator find(const key_t &key)
        {
            return iterator(BBST::find(&end_node_, key, comp_));
        }

        [[nodiscard]] const_iterator find(const key_t &key) const
        {
            return const_iterator(BBST::find(&end_node_, key, comp_));
        }

    };

}
#endif //BBST_AVL_TREE_H
