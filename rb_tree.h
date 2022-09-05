#ifndef BBST_RB_TREE_H
#define BBST_RB_TREE_H

#include <concepts>
#include <memory>
#include "tree_utils.h"

namespace BBST
{

    template<class rb_tree_node_ptr_t>
    uint32_t rb_subtree_invariant(rb_tree_node_ptr_t ptr)
    {
        if (ptr == nullptr)
            return 1;

        if (ptr->left != nullptr && ptr->left->parent != ptr)
            return 0;

        if (ptr->right != nullptr && ptr->right->parent != ptr)
            return 0;

        if (ptr->left != nullptr && ptr->right != nullptr && ptr->left == ptr->right)
            return 0;

        if (!ptr->is_black_)
        {
            if (ptr->left != nullptr && !ptr->left->is_black_)
                return 0;
            if (ptr->right != nullptr && !ptr->right->is_black_)
                return 0;
        }

        if (rb_subtree_invariant(ptr->left) + ptr->is_black_ != ptr->black_height_)
            return 0;

        if (rb_subtree_invariant(ptr->right) + ptr->is_black_ != ptr->black_height_)
            return 0;

        return ptr->black_height_;
    }

    template<class rb_tree_node_ptr_t>
    bool rb_tree_invariant(rb_tree_node_ptr_t root)
    {
        if (root == nullptr)
            return true;

        if (root->parent != nullptr || !root->is_black_ || !tree_is_left_child(root))
            return false;

        return rb_subtree_invariant(root) != 0;
    }

    template<class key_t, class mapped_t, class metadata_t>
    struct exposure
    {
        const key_t key;
        metadata_t metadata;
        mapped_t mapped;

        typedef const key_t key_type;
        typedef mapped_t mapped_type;
        typedef metadata_t metadata_type;

        exposure(key_t key_ = key_t(), metadata_t metadata_ = metadata_t(), mapped_t mapped_ = mapped_t())
                :
                key(std::move(key_))
                , metadata(std::move(metadata_))
                , mapped(std::move(mapped_))
        {}
    };

    //TODO: https://stackoverflow.com/questions/31623423/why-does-libcs-implementation-of-map-use-this-union
    template<class exposure_t>
    struct rb_tree_node : public base_tree_node<rb_tree_node<exposure_t>>
    {

        //expose type info
        typedef typename tree_node_base_traits<rb_tree_node>::value_type value_type;
        typedef typename tree_node_base_traits<rb_tree_node>::key_type key_type;
        typedef typename tree_node_base_traits<rb_tree_node>::metadata_type metadata_type;
        bool is_black_;
        uint32_t black_height_;
        value_type value_;

        template<class... Args>
        rb_tree_node(Args... args)
                :
                is_black_(false)
                , black_height_(1)
                , value_(std::forward<Args>(args)...)
        {}

        ~rb_tree_node()
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
            return value().key;
        }

        inline metadata_type &metadata() noexcept
        {
            return value().metadata;
        }
    };

    template<class exposure_t>
    struct tree_node_base_traits<rb_tree_node<exposure_t>>
    {
        typedef exposure_t value_type;
        typedef rb_tree_node<exposure_t> impl_type;
        typedef typename exposure_t::key_type key_type;
        typedef typename exposure_t::mapped_type mapped_type;
        typedef typename exposure_t::metadata_type metadata_type;
    };

    template<class Key, class Mapped, class Metadata, class MetadataUpdator, class Compare = std::less<Key>>
    requires (std::predicate<const Compare &, const Key &, const Key &> &&
              std::regular_invocable<const MetadataUpdator &, rb_tree_node<BBST::exposure<Key, Mapped, Metadata>> *>)
    class RedBlackTree
    {
    private:
        typedef rb_tree_node<BBST::exposure<Key, Mapped, Metadata>> rb_tree_node_t;
        typedef base_tree_node<rb_tree_node_t> base_tree_node_t;
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef rb_tree_node_t *rb_tree_node_ptr_t;
        typedef typename rb_tree_node_t::value_type value_type;
    public:
        typedef iterator_<base_tree_node_t> iterator;
        typedef const_iterator_<base_tree_node_t> const_iterator;

    private:

        base_tree_node_t end_node;
        base_tree_node_ptr_t begin_node;
        uint32_t size;
        Compare comp;
        MetadataUpdator updator;

        //TODO: templatize with allocator https://stackoverflow.com/questions/65262899/what-is-the-purpose-of-pointer-rebind
        template<class... Args>
        rb_tree_node_ptr_t construct_node(Args... args)
        {
            return new rb_tree_node_t(std::forward<Args>(args)...);
        }

        //TODO: templatize with allocator
        void destruct_node(const_iterator p)
        {
            delete static_cast<rb_tree_node_ptr_t>(p.get());
        }

        /*
         * sub_tree invariant: all condition is satisfied, except root might be red
         * Pre-condition: ptr == root or root is ancestor of ptr
         * Let ptr_height = rb_subtree_invariant(ptr), then
         *          ptr_height != 0 and (ptr == root || (ptr_height + ptr->parent->is_black_ = ptr->parent->black_height_)
         *
         * Post-condition: rb_subtree_invariant(root) != 0
         * This function is unaware of the existence of root->parent, and might return the new "rotated" root
         * It's the caller's responsibility to rewire the old root-parent to this possibly new root
         */
//        rb_tree_node_ptr_t rebalance_after_insert(rb_tree_node_ptr_t ptr
//                                                  , rb_tree_node_ptr_t root) noexcept(std::is_nothrow_invocable_v<const MetadataUpdator &, rb_tree_node<BBST::exposure<Key, Mapped, Metadata>> *>)
//        {
//
//        }

        void rebalance_after_insert(rb_tree_node_ptr_t ptr, rb_tree_node_ptr_t root) noexcept
        {
            ASSERT(rb_subtree_invariant(ptr), "first precondition failed");
            ASSERT(ptr == root ||
                   (rb_subtree_invariant(ptr) + ptr->parent_unsafe()->is_black_ == ptr->parent_unsafe()->black_height_),
                   "second precondition failed");
            while (ptr != root && !ptr->parent_unsafe()->is_black_)
            {
                //ptr->parent is not root
                if (tree_is_left_child(ptr->parent))
                {
                    rb_tree_node_ptr_t ptrUncle = ptr->parent->parent->right;
                    /*
                     *     G(B)                 G(R) <- next C
                     *    /    \               /    \
                     *   P(R)   U(R)   ->     P(B)   U(B)
                     *  /                    /
                     * C(R)                 C(R)
                     */
                    if (ptrUncle != nullptr && !ptrUncle->is_black_)
                    {
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = true;
                        ptr->black_height_++;
                        updator(ptr);
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = false;
                        ptrUncle->is_black_ = true;
                        ptrUncle->black_height_++;
                        updator(ptr);
                    }
                    else
                    {
                        /*
                         *     G(B)                 G(B)
                         *    /    \               /    \
                         *   P(R)   U(R)   ->     C(R)   U(R)
                         *    \                  /
                         *     C(R)             P(R) <- after rotate ptr point here
                         */
                        if (!tree_is_left_child(ptr))
                        {
                            ptr = ptr->parent_unsafe();
                            unguarded_tree_left_rotate(ptr);
                            updator(ptr);
                        }
                        /*
                         *      G(B)                  G(R)                  P(B)
                         *     /    \                /    \                /    \
                         *    P(R)   U(B)   ->      P(B)   U(B)     ->    C(R)   G(R)  <- ptr points here (end loop)
                         *   /                     /                              \
                         *  C(R)                  C(R)                             U(B)
                         */
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = true;
                        ptr->black_height_++;
                        updator(ptr);
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = false;
                        ptr->black_height_--;
                        tree_right_rotate(ptr);
                        updator(ptr->right);
                        updator(ptr);
                        break;
                    }
                }
                else
                {
                    rb_tree_node_ptr_t ptrUncle = ptr->parent->parent->left;
                    /*
                     *     G(B)                 G(R) <- next C
                     *    /    \               /    \
                     *   U(R)   P(R)   ->     U(B)   P(B)
                     *           \                    \
                     *            C(R)                 C(R)
                     */
                    if (ptrUncle != nullptr && !ptrUncle->is_black_)
                    {
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = true;
                        ptr->black_height_++;
                        updator(ptr);
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = false;
                        ptrUncle->is_black_ = true;
                        ptrUncle->black_height_++;
                        updator(ptr);
                    }
                    else
                    {
                        /*
                         *     G(B)                 G(B)
                         *    /    \               /    \
                         *   U(B)   P(R)   ->     U(B)   C(R)
                         *         /                      \
                         *        C(R)                     P(R) <- after rotate ptr point here
                         */
                        if (tree_is_left_child(ptr))
                        {
                            ptr = ptr->parent_unsafe();
                            tree_right_rotate(ptr);
                            updator(ptr);
                        }
                        /*
                         *      G(B)                  G(R)                                               P(B)
                         *     /    \                /    \                                             /    \
                         *    U(B)   P(R)   ->      U(R)   P(B)     ->    ptr points here(end loop) -> G(R)   C(R)
                         *            \                     \                                         /
                         *             C(R)                  C(R)                                    U(B)
                         */
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = true;
                        ptr->black_height_++;
                        updator(ptr);
                        ptr = ptr->parent_unsafe();
                        ptr->is_black_ = false;
                        ptr->black_height_--;
                        unguarded_tree_left_rotate(ptr);
                        updator(ptr->right);
                        updator(ptr);
                        break;
                    }
                }
            }
            while (true)
            {
                if (ptr == root) break;
                ptr = ptr->parent_unsafe();
                updator(ptr);
            }
            ASSERT(rb_subtree_invariant(ptr), "post-condition failed");
        }

        void
        insert_node_at(base_tree_node_ptr_t parent, rb_tree_node_ptr_t &child, rb_tree_node_ptr_t new_node) noexcept
        {
            new_node->left = nullptr;
            new_node->right = nullptr;
            new_node->parent = parent;
            child = new_node;
            if (begin_node->left != nullptr)
                begin_node = begin_node->left;
            rebalance_after_insert(new_node, end_node.left);
            rb_tree_node_ptr_t root = end_node.left;
            if (!root->is_black_)
            {
                root->is_black_ = true;
                root->black_height_++;
            }
        }

        std::pair<rb_tree_node_ptr_t &, base_tree_node_ptr_t> find_equal_or_insert_pos(const Key &key)
        {
            rb_tree_node_ptr_t *parent_link = &(end_node.left);
            rb_tree_node_ptr_t parent_link_point_to = *parent_link;
            if (parent_link_point_to != nullptr)
            {
                while (true)
                {
                    if (value_comp()(key, parent_link_point_to->value_.key))
                    {
                        if (parent_link_point_to->left != nullptr)
                        {
                            parent_link = &(parent_link_point_to->left);
                            parent_link_point_to = parent_link_point_to->left;
                        }
                        else
                        {
                            return {parent_link_point_to->left, parent_link_point_to};
                        }
                    }
                    else if (value_comp()(parent_link_point_to->value_.key, key))
                    {
                        if (parent_link_point_to->right != nullptr)
                        {
                            parent_link = &(parent_link_point_to->right);
                            parent_link_point_to = parent_link_point_to->right;
                        }
                        else
                        {
                            return {parent_link_point_to->right, parent_link_point_to};
                        }
                    }
                    else
                    {
                        return {*parent_link, parent_link_point_to};
                    }
                }
            }
            return {*parent_link, &end_node};
        }

        template<class... Args>
        std::pair<iterator, bool> emplace_key_args(Key key, Args... args)
        {

            auto [child, parent] = find_equal_or_insert_pos(key);
            if (child == nullptr)
            {
                rb_tree_node_ptr_t new_node = construct_node(std::forward<Key>(key), std::forward<Args>(args)...);
                insert_node_at(parent, child, new_node);
                return {iterator(new_node), true};
            }
            return {iterator(child), false};
        }

        template<class... Args>
        std::pair<iterator, bool> emplace_args(Args...args)
        {
            std::unique_ptr<rb_tree_node_t> new_node = std::make_unique<>(std::forward<Args>(args)...);
            auto [child, parent] = find_equal_or_insert_pos(new_node.get()->value_.key);
            if (child == nullptr)
            {
                insert_node_at(parent, child, new_node.release());
                return {new_node, true};
            }

            return {*child, false};
        }

        //        rb_tree_node_ptr
    public:
        //        RedBlackTree();

        RedBlackTree(MetadataUpdator updator_ = MetadataUpdator(), Compare comp_ = Compare())
                :
                size(0)
                , end_node(nullptr, nullptr, nullptr)
                , begin_node(&end_node)
                , updator(std::move(updator_))
                , comp(std::move(comp_))
        {

        }

        ~RedBlackTree()
        {
            delete end_node.left;
        }

        inline iterator begin()
        {
            return iterator(begin_node);
        }

        [[nodiscard]] inline const_iterator begin() const
        {
            return const_iterator(begin_node);
        }

        inline iterator end()
        {
            return iterator(&end_node);
        }

        [[nodiscard]] inline const_iterator end() const
        {
            return const_iterator(&end_node);
        }

        inline Compare &value_comp() noexcept
        {
            return comp;
        }

        [[nodiscard]] inline const Compare &value_comp() const noexcept
        {
            return comp;
        }

        iterator lower_bound(const Key &key)
        {
            return BBST::lower_bound(end(), key, value_comp());
        }

        [[nodiscard]] const_iterator lower_bound(const Key &key) const
        {
            return BBST::lower_bound(end(), key, value_comp());
        }

        iterator find(const Key &key)
        {
            return BBST::find(end(), key, value_comp());
        }

        [[nodiscard]] const_iterator find(const Key &key) const
        {
            return BBST::find(end(), key, value_comp());
        }

        //key,metadata,mapped
        template<class... Args>
        inline std::pair<iterator, bool> try_emplace(Key key, Args...args)
        {
            return emplace_key_args(std::forward<Key>(key), std::forward<Args>(args)...);
        }

        value_type &operator[](const Key &key)
        {
            return emplace_key_args(key).first.operator*();
        }

        value_type &operator[](Key &&key)
        {
            return emplace_key_args(std::move(key)).first.operator*();
        }

        void tree_remove(rb_tree_node_ptr_t z) noexcept
        {
            rb_tree_node_ptr_t root = end_node->left;
            // z will be removed from the tree.  Client still needs to destruct/deallocate it
            // y is either z, or if z has two children, tree_next(z).
            // y will have at most one child.
            // y will be the initial hole in the tree (make the hole at a leaf)
            rb_tree_node_ptr_t y;
            if (z->left == nullptr || z->right == nullptr)
            {
                y = z;
            }
            else
            {
                y = z->right;
                while (y->left != nullptr) y = y->left;
            }
            // x is y's possibly null single child
            rb_tree_node_ptr_t x = y->left != nullptr ? y->left : y->right;
            // w is x's possibly null uncle (will become x's sibling)
            rb_tree_node_ptr_t w = nullptr;
            // link x to y's parent, and find w
            if (x != nullptr)
                x->parent = y->parent;
            if (tree_is_left_child(y))
            {
                y->parent->left = x;
                if (y != root)
                    w = y->parent->right;
                else
                    root = x;  // w == nullptr
            }
            else
            {
                y->parent->right = x;
                // y can't be root if it is a right child
                w = y->parent->left;
            }
            bool removed_black = y->is_black_;
            // If we didn't remove z, do so now by splicing in y for z,
            //    but copy z's color.  This does not impact x or w.
            if (y != z)
            {
                // z->left != nullptr but z->right might == x == nullptr
                y->parent = z->parent;
                if (tree_is_left_child(z))
                    y->parent->left = y;
                else
                    y->parent->right = y;
                y->left = z->left;
                y->left->parent = y;
                y->right = z->right;
                if (y->right != nullptr)
                    y->right->parent = y;
                y->is_black_ = z->is_black_;
                if (root == z)
                    root = y;
            }
            // There is no need to rebalance if we removed a red, or if we removed
            //     the last node.
            if (removed_black && root != nullptr)
            {
                // Rebalance:
                // x has an implicit black color (transferred from the removed y)
                //    associated with it, no matter what its color is.
                // If x is root (in which case it can't be null), it is supposed
                //    to be black anyway, and if it is doubly black, then the double
                //    can just be ignored.
                // If x is red (in which case it can't be null), then it can absorb
                //    the implicit black just by setting its color to black.
                // Since y was black and only had one child (which x points to), x
                //   is either red with no children, else null, otherwise y would have
                //   different black heights under left and right pointers.
                // if (x == root || x != nullptr && !x->isBlack)
                if (x != nullptr)
                    x->is_black_ = true;
                else
                {
                    //  Else x isn't root, and is "doubly black", even though it may
                    //     be null.  w can not be null here, else the parent would
                    //     see a black height >= 2 on the x side and a black height
                    //     of 1 on the w side (w must be a non-null black or a red
                    //     with a non-null black child).
                    while (true)
                    {
                        if (!tree_is_left_child(w))  // if x is left child
                        {
                            if (!w->is_black_)
                            {
                                w->is_black_ = true;
                                parent_unsafe(w)->is_black_ = false;
                                unguarded_tree_left_rotate(parent_unsafe(w));
                                // x is still valid
                                // reset root only if necessary
                                if (root == w->left)
                                    root = w;
                                // reset sibling, and it still can't be null
                                w = w->left->right;
                            }
                            // w->isBlack is now true, w may have null children
                            if ((w->left == nullptr || w->left->is_black_) &&
                                (w->right == nullptr || w->right->is_black_))
                            {
                                w->is_black_ = false;
                                x = parent_unsafe(w);
                                // x can no longer be null
                                if (x == root || !x->is_black_)
                                {
                                    x->is_black_ = true;
                                    break;
                                }
                                // reset sibling, and it still can't be null
                                w = tree_is_left_child(x) ? x->parent->right : x->parent->left;
                                // continue;
                            }
                            else  // w has a red child
                            {
                                if (w->right == nullptr || w->right->is_black_)
                                {
                                    // w left child is non-null and red
                                    w->left->is_black_ = true;
                                    w->is_black_ = false;
                                    tree_right_rotate(w);
                                    // w is known not to be root, so root hasn't changed
                                    // reset sibling, and it still can't be null
                                    w = parent_unsafe(w);
                                }
                                // w has a right red child, left child may be null
                                w->is_black_ = parent_unsafe(w)->is_black_;
                                parent_unsafe(w)->is_black_ = true;
                                w->right->is_black_ = true;
                                unguarded_tree_left_rotate(parent_unsafe(w));
                                break;
                            }
                        }
                        else
                        {
                            if (!w->is_black_)
                            {
                                w->is_black_ = true;
                                parent_unsafe(w)->is_black_ = false;
                                tree_right_rotate(parent_unsafe(w));
                                // x is still valid
                                // reset root only if necessary
                                if (root == w->right)
                                    root = w;
                                // reset sibling, and it still can't be null
                                w = w->right->left;
                            }
                            // w->isBlack is now true, w may have null children
                            if ((w->left == nullptr || w->left->is_black_) &&
                                (w->right == nullptr || w->right->is_black_))
                            {
                                w->is_black_ = false;
                                x = parent_unsafe(w);
                                // x can no longer be null
                                if (!x->is_black_ || x == root)
                                {
                                    x->is_black_ = true;
                                    break;
                                }
                                // reset sibling, and it still can't be null
                                w = tree_is_left_child(x) ? x->parent->right : x->parent->left;
                                // continue;
                            }
                            else  // w has a red child
                            {
                                if (w->left == nullptr || w->left->is_black_)
                                {
                                    // w right child is non-null and red
                                    w->right->is_black_ = true;
                                    w->is_black_ = false;
                                    unguarded_tree_left_rotate(w);
                                    // w is known not to be root, so root hasn't changed
                                    // reset sibling, and it still can't be null
                                    w = parent_unsafe(w);
                                }
                                // w has a left red child, right child may be null
                                w->is_black_ = parent_unsafe(w)->is_black_;
                                parent_unsafe(w)->is_black_ = true;
                                w->left->is_black_ = true;
                                tree_right_rotate(parent_unsafe(w));
                                break;
                            }
                        }
                    }
                }
            }
        }
    };
}

#endif //BBST_RB_TREE_H
