#ifndef BBST_RB_TREE_H
#define BBST_RB_TREE_H

#include <concepts>
#include <memory>
#include "tree_utils.h"

namespace bbst
{
    template<class exposure_t>
    struct rb_tree_node;

    template<class key_t, class mapped_t, class metadata_t>
    struct rb_tree_header;

    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t>
    requires (std::predicate<const comparator_t &, const key_t &, const key_t &> &&
              std::regular_invocable<const metadata_updator_t &, rb_tree_node<bbst::exposure<key_t, mapped_t, metadata_t>> *>)
    class rb_tree;
}
//invariant debug
namespace bbst
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

        auto left_height = rb_subtree_invariant(ptr->left);
        if (left_height == 0)
            return 0;

        if (rb_subtree_invariant(ptr->right) != left_height)
            return 0;

        return left_height + ptr->is_black_;
    }

    template<class rb_tree_node_ptr_t>
    uint32_t rb_tree_invariant(rb_tree_node_ptr_t root)
    {
        if (root == nullptr)
            return 1;
        if (!root->is_black_)
            return 0;
        return rb_subtree_invariant(root);
    }

    template<class rb_tree_header_t>
    bool rb_tree_header_invariant(rb_tree_header_t header)
    {
        auto black_height = rb_tree_invariant(header.root_);
        if (black_height == 0)
            return false;
        if (black_height != header.black_height_)
            return false;
        return true;
    }
}

//rb_tree_node
namespace bbst
{

    template<class exposure_t>
    struct rb_tree_node : public base_tree_node<rb_tree_node<exposure_t>>
    {

        //expose type info
        typedef typename tree_node_base_traits<rb_tree_node>::value_type value_type;
        typedef typename tree_node_base_traits<rb_tree_node>::key_type key_type;
        typedef typename tree_node_base_traits<rb_tree_node>::metadata_type metadata_type;
        bool is_black_//: 1
        ;
        value_type value_;

        template<class... Args>
        explicit rb_tree_node(Args... args)
                :
                is_black_(false)
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
    struct tree_node_base_traits<rb_tree_node<exposure_t>>
    {
        typedef exposure_t value_type;
        typedef rb_tree_node<exposure_t> impl_type;
        typedef typename exposure_t::key_type key_type;
        typedef typename exposure_t::mapped_type mapped_type;
        typedef typename exposure_t::metadata_type metadata_type;
    };

}

//rb_tree_header
namespace bbst
{

    //header is non-owning type, mainly used as a bookkeeping struct for join-split operator
    template<class key_t, class mapped_t, class metadata_t>
    struct rb_tree_header
    {
    private:
        typedef rb_tree_node<bbst::exposure<key_t, mapped_t, metadata_t>> rb_tree_node_t;
        typedef rb_tree_node_t *rb_tree_node_ptr_t;
        typedef base_tree_node<rb_tree_node_t> base_tree_node_t;
        typedef rb_tree_header<key_t, mapped_t, metadata_t> rb_tree_header_t;

    public:
        rb_tree_node_ptr_t root_;
        uint32_t black_height_;

        rb_tree_header(rb_tree_node_ptr_t root, uint32_t black_height)
                :
                root_(root)
                , black_height_(black_height)
        {}

        static inline rb_tree_header empty_header()
        {
            return {nullptr, 1};
        }

        [[nodiscard]] bool empty() const
        {
            return root_ == nullptr;
        }

        template<class key_holder_t, class mapped_holder_t, class metadata_holder_t, class metadata_updator_holder_t, class comparator_holder_t, class tag_holder_t> friend
        class rb_tree_custom_invoke;
    };
}

//helper
namespace bbst
{
    /*
     * Pre-condition: ptr must be red. root is either ptr or black
     *                root is ptr's ancestor
     *                red black subtree at ptr's invariant must be valid (yes please call updator for ptr before calling me)
     *                the only possible violation of subtree at root is two consecutive red at ptr
     * Post-condition: rb_subtree_invariant(root) != 0
     * This function is unaware of the existence of root->parent, and might return the new "rotated" root
     * It's the caller's responsibility to link the old root->parent to this possibly new root
     */
    template<class rb_tree_node_ptr_t, class metadata_updator_t>
    rb_tree_node_ptr_t rb_tree_insert_fixup(rb_tree_node_ptr_t ptr, rb_tree_node_ptr_t root
                                            , const metadata_updator_t &updator_) noexcept(std::is_nothrow_invocable_v<const metadata_updator_t &, rb_tree_node_ptr_t>)
    {
        ASSERT(!ptr->is_black_ && (ptr == root || (rb_subtree_invariant(ptr->parent->left) == rb_subtree_invariant(ptr->parent->right) &&
                                                   root->is_black_)), "precondition failed");
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
                    updator_(ptr);//update U
                    ptr = ptr->parent_unsafe();
                    ptr->is_black_ = false;
                    ptrUncle->is_black_ = true;
                    updator_(ptr);//update G
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
                        updator_(ptr);//update P
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
                    //                    updator_(ptr); // unnecessary update P
                    ptr = ptr->parent_unsafe();
                    ptr->is_black_ = false;
                    bool is_finish = ptr == root;
                    if (is_finish)
                        tree_root_right_rotate(ptr);
                    else
                        unguarded_tree_right_rotate(ptr);
                    updator_(ptr); //update G
                    ptr = ptr->parent_unsafe();
                    updator_(ptr); //update P
                    if (is_finish)
                    {
                        ASSERT(rb_tree_invariant(ptr), "post condition false");
                        return ptr;
                    }
                    break;
                }
            }
            else
            {
                rb_tree_node_ptr_t ptrUncle = ptr->parent->parent->left;
                /*
                 *     G(B)                 G2(R) <- next C
                 *    /    \               /    \
                 *   U(R)   P(R)   ->     U(B)   P(B)
                 *           \                    \
                 *            C(R)                 C(R)
                 */
                if (ptrUncle != nullptr && !ptrUncle->is_black_)
                {
                    ptr = ptr->parent_unsafe();
                    ptr->is_black_ = true;
                    updator_(ptr); //update P
                    ptr = ptr->parent_unsafe();
                    ptr->is_black_ = false;
                    ptrUncle->is_black_ = true;
                    updator_(ptr); // update G
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
                        unguarded_tree_right_rotate(ptr);
                        updator_(ptr);//update P
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
                    //                    updator_(ptr);//unnecessary
                    ptr = ptr->parent_unsafe();
                    ptr->is_black_ = false;
                    bool is_finish = ptr == root;
                    if (is_finish)
                        tree_root_left_rotate(ptr);
                    else
                        unguarded_tree_left_rotate(ptr);
                    updator_(ptr);//update G
                    ptr = ptr->parent_unsafe();
                    updator_(ptr);//update P
                    if (is_finish)
                    {
                        ASSERT(rb_tree_invariant(ptr), "post condition failed");
                        return ptr;
                    }
                    break;
                }
            }
        }
        while (ptr != root)
        {
            ptr = ptr->parent_unsafe();
            updator_(ptr);
        }
        ASSERT(rb_subtree_invariant(ptr), "post condition failed");
        return ptr;
    }

    template<class rb_tree_header_t, class rb_tree_node_ptr_t, class metadata_updator_t, class comparator_t>
    rb_tree_header_t rb_tree_join_x(rb_tree_header_t left, rb_tree_node_ptr_t x, rb_tree_header_t right, const metadata_updator_t &metadata_updator
                                    , const comparator_t &comparator) noexcept(std::is_nothrow_invocable_v<const metadata_updator_t &, rb_tree_node_ptr_t>)
    {
        ASSERT(rb_tree_header_invariant(left), "left header invariant false");
        ASSERT(left.root_ == nullptr || comparator(bbst::tree_max(left.root_)->key(), x->key()), "left tree must be less than x");
        ASSERT(rb_tree_header_invariant(right), "right header invariant false");
        ASSERT(right.root_ == nullptr || comparator(x->key(), bbst::tree_min(right.root_)->key()), "right tree must be greater than x");
        if (left.black_height_ == right.black_height_)
        {
            x->left = left.root_;
            if (left.root_)
                left.root_->parent = x;
            x->right = right.root_;
            if (right.root_)
                right.root_->parent = x;
            x->is_black_ = true;
            left.root_ = x;
            left.black_height_++;
            metadata_updator(x);
            ASSERT(rb_tree_header_invariant(left), "post condition failed");
            return left;
        }
        else if (left.black_height_ < right.black_height_)
        {
            rb_tree_node_ptr_t ptr = right.root_;
            uint32_t diff = right.black_height_ - left.black_height_;
            while (true)
            {
                if ((ptr->left == nullptr || ptr->left->is_black_) && --diff == 0)
                    break;
                ptr = ptr->left;
            }
            x->left = left.root_;
            if (x->left)
                x->left->parent = x;
            x->right = ptr->left;
            if (x->right)
                x->right->parent = x;
            x->is_black_ = false;
            ptr->left = x;
            x->parent = ptr;
            metadata_updator(x);
            right.root_ = rb_tree_insert_fixup(x, right.root_, metadata_updator);
            if (!right.root_->is_black_)
            {
                right.root_->is_black_ = true;
                right.black_height_++;
            }
            ASSERT(rb_tree_header_invariant(right), "post condition failed");
            return right;
        }
        else
        {
            rb_tree_node_ptr_t ptr = left.root_;
            uint32_t diff = left.black_height_ - right.black_height_;
            while (true)
            {
                if ((ptr->right == nullptr || ptr->right->is_black_) && --diff == 0)
                    break;
                ptr = ptr->right;
            }
            x->right = right.root_;
            x->left = ptr->right;
            if (x->right)
                x->right->parent = x;
            if (x->left)
                x->left->parent = x;
            x->is_black_ = false;
            ptr->right = x;
            x->parent = ptr;
            metadata_updator(x);
            left.root_ = rb_tree_insert_fixup(x, left.root_, metadata_updator);
            if (!left.root_->is_black_)
            {
                left.root_->is_black_ = true;
                left.black_height_++;
            }
            ASSERT(rb_tree_header_invariant(left), "post condition failed");
            return left;
        }
    }
}

//rb_tree
namespace bbst
{
    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t = std::less<key_t>>
    requires (std::predicate<const comparator_t &, const key_t &, const key_t &> &&
              std::regular_invocable<const metadata_updator_t &, rb_tree_node<bbst::exposure<key_t, mapped_t, metadata_t>> *>)
    class rb_tree
    {
    private:
        typedef rb_tree_node<exposure<key_t, mapped_t, metadata_t>> rb_tree_node_t;
        typedef base_tree_node<rb_tree_node_t> base_tree_node_t;
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef rb_tree_node_t *rb_tree_node_ptr_t;
        typedef rb_tree_header<key_t, mapped_t, metadata_t> rb_tree_header_t;
        typedef typename rb_tree_node_t::value_type value_type;
    public:
        typedef tree_bidirectional_iterator_<base_tree_node_t> iterator;
        typedef tree_bidirectional_const_iterator_<base_tree_node_t> const_iterator;
    private:

        base_tree_node_t end_node_;
        base_tree_node_ptr_t begin_node_;
        comparator_t comp_;
        metadata_updator_t updator_;
        uint32_t black_height_;

        std::pair<rb_tree_node_ptr_t &, base_tree_node_ptr_t> inline find_equal_or_insert_pos(const key_t &key)
        {
            return bbst::find_equal_or_insert_pos<key_t, base_tree_node_ptr_t, rb_tree_node_ptr_t, comparator_t>(key, &end_node_, comp_);
        }

        void insert_node_at(base_tree_node_ptr_t parent, rb_tree_node_ptr_t &child, rb_tree_node_ptr_t new_node) noexcept
        {
            new_node->left = nullptr;
            new_node->right = nullptr;
            new_node->parent = parent;
            child = new_node;
            updator_(new_node);
            if (begin_node_->left != nullptr)
                begin_node_ = begin_node_->left;
            rb_tree_node_ptr_t root = (end_node_.left = rb_tree_insert_fixup(new_node, end_node_.left, updator_));
            if (!root->is_black_)
            {
                root->is_black_ = true;
                black_height_++;
            }
        }

        template<class... Args>
        std::pair<iterator, bool> emplace_key_args(key_t key, Args... args)
        {

            auto [child, parent] = find_equal_or_insert_pos(key);
            if (child == nullptr)
            {
                rb_tree_node_ptr_t new_node = construct_node(std::forward<key_t>(key), std::forward<Args>(args)...);
                insert_node_at(parent, child, new_node);
                return {iterator(new_node), true};
            }
            return {iterator(child), false};
        }

        template<class... Args>
        std::pair<iterator, bool> emplace_args(Args...args)
        {
            std::unique_ptr<rb_tree_node_t> new_node = std::make_unique<>(std::forward<Args>(args)...);
            auto [child, parent] = find_equal_or_insert_pos(new_node->value_.key);
            if (child == nullptr)
            {
                insert_node_at(parent, child, new_node.release());
                return {new_node, true};
            }

            return {*child, false};
        }

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

        rb_tree(rb_tree_header_t header, const metadata_updator_t &updator, const comparator_t &comp)
                :
                black_height_(header.black_height_)
                , end_node_(nullptr, header.root_, nullptr)
                , begin_node_(header.root_ == nullptr ? &end_node_ : tree_min(header.root_))
                , comp_(comp)
                , updator_(updator)
        {
            if (header.root_ != nullptr) header.root_->parent = &end_node_;
        }

    public:
        template<class comparator_forward_t=comparator_t, class metadata_updator_forward_t=metadata_updator_t>
        requires (std::is_same_v<comparator_t, std::decay_t<comparator_forward_t>> &&
                  std::is_same_v<metadata_updator_t, std::decay_t<metadata_updator_forward_t>>)
        rb_tree(metadata_updator_forward_t &&updator = metadata_updator_t(), comparator_forward_t &&comp = comparator_t())
                :
                end_node_(nullptr, nullptr, nullptr)
                , begin_node_(&end_node_)
                , updator_(std::forward<metadata_updator_forward_t>(updator))
                , comp_(std::forward<comparator_forward_t>(comp))
                , black_height_(1)
        {

        }

        rb_tree(rb_tree &&other) noexcept(std::is_nothrow_move_constructible_v<comparator_t> && std::is_nothrow_move_constructible_v<metadata_updator_t>)
                :
                end_node_(nullptr, std::exchange(other.end_node_.left, nullptr), nullptr)
                , begin_node_(other.begin_node_ == &other.end_node_ ? &end_node_ : std::exchange(other.begin_node_, &other.end_node_))
                , black_height_(std::exchange(other.black_height_, 1))
                , comp_(std::move(other.comp_))
                , updator_(std::move(other.updator_))
        {
            if (end_node_.left) end_node_.left->parent = &end_node_;
        }

        ~rb_tree()
        {
            delete end_node_.left;
        }

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
            return iterator(bbst::lower_bound(&end_node_, key, comp_));
        }

        [[nodiscard]] const_iterator lower_bound(const key_t &key) const
        {
            return const_iterator(bbst::lower_bound(&end_node_, key, comp_));
        }

        iterator find(const key_t &key)
        {
            return iterator(bbst::find(&end_node_, key, comp_));
        }

        [[nodiscard]] const_iterator find(const key_t &key) const
        {
            return const_iterator(bbst::find(&end_node_, key, comp_));
        }

        [[nodiscard]] bool empty() const
        {
            return begin_node_ == &end_node_;
        }

        //key,metadata,mapped
        template<class... Args>
        inline std::pair<iterator, bool> try_emplace(key_t key, Args...args)
        {
            return emplace_key_args(std::forward<key_t>(key), std::forward<Args>(args)...);
        }

        template<class key_forward_t>
        requires std::is_same_v<key_t, std::remove_reference_t<key_forward_t>>
        mapped_t &operator[](key_forward_t &&key)
        {
            return emplace_key_args(std::forward<key_forward_t>(key)).first->mapped;
        }

        //TODO:
        void tree_remove(rb_tree_node_ptr_t z) noexcept
        {
            rb_tree_node_ptr_t root = end_node_->left;
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
                while (y->left != nullptr)
                    y = y->left;
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
                            if ((w->left == nullptr || w->left->is_black_) && (w->right == nullptr || w->right->is_black_))
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
                                    unguarded_tree_right_rotate(w);
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
                                unguarded_tree_right_rotate(parent_unsafe(w));
                                // x is still valid
                                // reset root only if necessary
                                if (root == w->right)
                                    root = w;
                                // reset sibling, and it still can't be null
                                w = w->right->left;
                            }
                            // w->isBlack is now true, w may have null children
                            if ((w->left == nullptr || w->left->is_black_) && (w->right == nullptr || w->right->is_black_))
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
                                unguarded_tree_right_rotate(parent_unsafe(w));
                                break;
                            }
                        }
                    }
                }
            }
        }

        template<class key_holder_t, class mapped_holder_t, class metadata_holder_t, class metadata_updator_holder_t, class comparator_holder_t, class tag> friend
        class rb_tree_custom_invoke;
    };

}

#endif //BBST_RB_TREE_H
