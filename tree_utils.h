#ifndef BBST_TREE_UTILS_H
#define BBST_TREE_UTILS_H

#include <cassert>
#include <utility>
#include <concepts>
#include <iostream>

//debug assert
namespace BBST
{
#ifndef NDEBUG
#define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << (message) << std::endl; \
            std::abort(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif
}

//pointer template
namespace BBST
{

    template<class base_tree_node_t>
    base_tree_node_t tree_next_iter(base_tree_node_t ptr) noexcept
    {
        if (ptr->right != nullptr)
        {
            ptr = ptr->right;
            while (ptr->left != nullptr)
                ptr = ptr->left;
            return ptr;
        }
        while (!tree_is_left_child(ptr))
            ptr = ptr->parent;
        return ptr->parent;
    }

    template<class base_tree_node_ptr_t>
    inline base_tree_node_ptr_t tree_prev_iter(base_tree_node_ptr_t ptr) noexcept
    {
        if (ptr->left != nullptr)
        {
            ptr = ptr->left;
            while (ptr->right != nullptr)
                ptr = ptr->right;
            return ptr;
        }
        while (tree_is_left_child(ptr))
            ptr = ptr->parent;
        return ptr->parent;
    }

    //TODO: https://docs.microsoft.com/en-us/cpp/standard-library/sample-container-class?view=msvc-170
    template<class base_tree_node_t>
    class tree_forward_iterator_
    {
    public:
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef std::forward_iterator_tag iterator_category;

    private:
        base_tree_node_ptr_t ptr;

        typedef typename base_tree_node_t::impl_type impl_type;

    public:
        typedef typename impl_type::value_type value_type;
        typedef base_tree_node_ptr_t pointer;

        inline tree_forward_iterator_(base_tree_node_ptr_t ptr_) noexcept: ptr(ptr_)
        {}

        inline base_tree_node_t get() const noexcept
        {
            return ptr;
        }

        inline value_type &operator*() const
        {
            return ptr->self_downcast_unsafe()->value();
        }

        inline value_type *operator->() const
        {
            return &ptr->self_downcast_unsafe()->value();
        }

        inline tree_forward_iterator_ &operator++()
        {
            ptr = tree_next_iter(ptr);
            return *this;
        }

        inline tree_forward_iterator_ operator++(int)
        {
            tree_forward_iterator_ temp(*this);
            ++(*this);
            return temp;
        }

        friend inline bool operator==(const tree_forward_iterator_ &lhs, const tree_forward_iterator_ &rhs)
        {
            return lhs.ptr == rhs.ptr;
        }

        friend inline bool operator!=(const tree_forward_iterator_ &lhs, const tree_forward_iterator_ &rhs)
        {
            return lhs.ptr != rhs.ptr;
        }
    };

    template<class base_tree_node_t>
    class tree_forward_const_iterator_
    {
    public:
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef std::forward_iterator_tag iterator_category;

    private:
        base_tree_node_ptr_t ptr;

        typedef typename base_tree_node_t::impl_type impl_type;

    public:
        typedef typename impl_type::value_type value_type;
        typedef base_tree_node_ptr_t pointer;

        inline tree_forward_const_iterator_(base_tree_node_ptr_t ptr_) noexcept: ptr(ptr_)
        {}

        inline tree_forward_const_iterator_(tree_forward_iterator_<base_tree_node_t> o) : ptr(o.get())
        {}

        inline const base_tree_node_ptr_t get() const noexcept
        {
            return ptr;
        }

        inline const value_type &operator*() const
        {
            return ptr->self_downcast_unsafe()->value();
        }

        inline const value_type *operator->() const
        {
            return &ptr->self_downcast_unsafe()->value();
        }

        inline tree_forward_const_iterator_ &operator++()
        {
            ptr = tree_next_iter(ptr);
            return *this;
        }

        inline tree_forward_const_iterator_ operator++(int)
        {
            tree_forward_iterator_ temp(*this);
            ++(*this);
            return temp;
        }

        friend inline bool operator==(const tree_forward_const_iterator_ &lhs, const tree_forward_const_iterator_ &rhs)
        {
            return lhs.ptr == rhs.ptr;
        }

        friend inline bool operator!=(const tree_forward_const_iterator_ &lhs, const tree_forward_const_iterator_ &rhs)
        {
            return lhs.ptr != rhs.ptr;
        }
    };

    template<class base_tree_node_t>
    class tree_bidirectional_iterator_
    {
    public:
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef std::bidirectional_iterator_tag iterator_category;

    private:
        base_tree_node_ptr_t ptr;

        typedef typename base_tree_node_t::impl_type impl_type;

    public:
        typedef typename impl_type::value_type value_type;
        typedef base_tree_node_ptr_t pointer;

        inline tree_bidirectional_iterator_(base_tree_node_ptr_t ptr_) noexcept: ptr(ptr_)
        {}

        inline base_tree_node_t get() const noexcept
        {
            return ptr;
        }

        inline value_type &operator*() const
        {
            return ptr->self_downcast_unsafe()->value();
        }

        inline value_type *operator->() const
        {
            return &ptr->self_downcast_unsafe()->value();
        }

        inline tree_bidirectional_iterator_ &operator++()
        {
            ptr = tree_next_iter(ptr);
            return *this;
        }

        inline tree_bidirectional_iterator_ operator++(int)
        {
            tree_forward_iterator_ temp(*this);
            ++(*this);
            return temp;
        }

    public:
        inline tree_bidirectional_iterator_ &operator--()
        {
            this->ptr = tree_prev_iter(this->ptr);
            return *this;
        }

        inline tree_bidirectional_iterator_ operator--(int)
        {
            tree_bidirectional_iterator_ temp(*this);
            --(*this);
            return temp;
        }

        friend inline bool operator==(const tree_bidirectional_iterator_ &lhs, const tree_bidirectional_iterator_ &rhs)
        {
            return lhs.ptr == rhs.ptr;
        }

        friend inline bool operator!=(const tree_bidirectional_iterator_ &lhs, const tree_bidirectional_iterator_ &rhs)
        {
            return lhs.ptr != rhs.ptr;
        }
    };

    template<class base_tree_node_t>
    class tree_bidirectional_const_iterator_
    {
    public:
        typedef const base_tree_node_t *const_base_tree_node_ptr_t;
        typedef std::bidirectional_iterator_tag iterator_category;

    private:
        const_base_tree_node_ptr_t ptr;

        typedef typename base_tree_node_t::impl_type impl_type;

    public:
        typedef typename impl_type::value_type value_type;
        typedef const_base_tree_node_ptr_t pointer;

        explicit inline tree_bidirectional_const_iterator_(const_base_tree_node_ptr_t ptr_) noexcept: ptr(ptr_)
        {}

        inline tree_bidirectional_const_iterator_(tree_bidirectional_iterator_<base_tree_node_t> o) : ptr(o.get())
        {};

        inline const const_base_tree_node_ptr_t get() const noexcept
        {
            return ptr;
        }

        inline const value_type &operator*()
        {
            return ptr->self_downcast_unsafe()->value();
        }

        inline const value_type *operator->()
        {
            return &ptr->self_downcast_unsafe()->value();
        }

        inline tree_bidirectional_const_iterator_ &operator++()
        {
            ptr = tree_next_iter(ptr);
            return *this;
        }

        inline tree_bidirectional_const_iterator_ &operator--()
        {
            ptr = tree_prev_iter(ptr);
            return *this;
        }

        inline tree_bidirectional_const_iterator_ operator++(int)
        {
            tree_bidirectional_const_iterator_ temp(*this);
            ++(*this);
            return temp;
        }

        inline tree_bidirectional_const_iterator_ operator--(int)
        {
            tree_bidirectional_const_iterator_ temp(*this);
            --(*this);
            return temp;
        }

        friend inline bool operator==(const tree_bidirectional_const_iterator_ &lhs, const tree_bidirectional_const_iterator_ &rhs)
        {
            return lhs.ptr == rhs.ptr;
        }

        friend inline bool operator!=(const tree_bidirectional_const_iterator_ &lhs, const tree_bidirectional_const_iterator_ &rhs)
        {
            return lhs.ptr != rhs.ptr;
        }
    };

}

//tree rotate
namespace BBST
{
    template<class base_tree_node_ptr_t>
    inline bool tree_is_left_child(base_tree_node_ptr_t ptr) noexcept
    {
        return ptr == ptr->parent->left;
    }

    /*
    *      P              L
    *     / \            / \
    *    L   R    ->    LL  P
    *   / \                / \
    *  LL  LR             LR  R
    *  It's the caller responsibility to update invariant on P and L (P->parent)
    */
    template<class base_tree_node_ptr_t>
    void unguarded_tree_right_rotate(base_tree_node_ptr_t P) noexcept
    {
        base_tree_node_ptr_t L = P->left;
        P->left = L->right;
        if (P->left != nullptr)
            P->left->parent = P;
        L->parent = P->parent;

        if (tree_is_left_child(P))
            P->parent->left = L;
        else
            P->parent->right = L;

        L->right = P;
        P->parent = L;
    }

    /*
       *   P              R
       *  / \            / \
       * L   R    ->    P   RR
       *    / \        / \
       *   RL  RR     L   RL
       *  It's the caller responsibility to update invariant on P and R (P->parent)
       */
    template<class base_tree_node_ptr_t>
    void unguarded_tree_left_rotate(base_tree_node_ptr_t P) noexcept
    {
        base_tree_node_ptr_t R = P->right;
        P->right = R->left;

        if (P->right != nullptr)
            P->right->parent = P;
        R->parent = P->parent;

        if (tree_is_left_child(P))
            P->parent->left = R;
        else
            P->parent->right = R;

        R->left = P;
        P->parent = R;
    }

    /*
     * the only difference is this version doesn't assume the existence of P->parent
     */
    template<class base_tree_node_ptr_t>
    void tree_root_left_rotate(base_tree_node_ptr_t P) noexcept
    {
        base_tree_node_ptr_t R = P->right;
        P->right = R->left;
        if (P->right != nullptr)
            P->right->parent = P;
        R->parent = P->parent;
        R->left = P;
        P->parent = R;
    }

    /*
     * the only difference is this version doesn't assume the existence of P->parent
     */
    template<class base_tree_node_ptr_t>
    void tree_root_right_rotate(base_tree_node_ptr_t P) noexcept
    {
        base_tree_node_ptr_t L = P->left;
        P->left = L->right;
        if (P->left != nullptr)
            P->left->parent = P;
        L->parent = P->parent;
        L->right = P;
        P->parent = L;
    }

    /*
     *      X                       X                                    Y
     *     / \                     / \                                 /   \
     *    t1  Z                   t1  Y                               X      Z
     *       / \        ->           / \            ->              /   \  /   \
     *      Y   t4                  t2  Z                          t1   t2 t3  t4
     *     / \                         / \
     *    t2  t3                      t3  t4
     */
    template<class base_tree_node_ptr_t>
    base_tree_node_ptr_t unguarded_tree_right_left_rotate(base_tree_node_ptr_t X, base_tree_node_ptr_t Z)
    {
        base_tree_node_ptr_t Y = Z->left;
        X->right = Y->left;
        if (X->right) X->right->parent = X;
        Z->left = Y->right;
        if (Z->left) Z->left->parent = Z;
        Y->right = Z;
        Z->parent = Y;
        Y->parent = X->parent;
        if (tree_is_left_child(X))
            X->parent->left = Y;
        else
            X->parent->right = Y;
        Y->left = X;
        X->parent = Y;
        return Y;
    }

    /*
     *          X                           X                            Y
     *         / \                         / \                         /   \
     *        Z   t4                      Y   t4                      Z     X
     *       / \            ->           / \                ->       / \   / \
     *      t1  Y                       Z  t3                       t1 t2 t3 t4
     *         / \                     / \
     *        t2 t3                   t1 t2
     */
    template<class base_tree_node_ptr_t>
    base_tree_node_ptr_t unguarded_tree_left_right_rotate(base_tree_node_ptr_t X, base_tree_node_ptr_t Z)
    {
        base_tree_node_ptr_t Y = Z->right;
        X->left = Y->right;
        if (X->left) X->left->parent = X;
        Z->right = Y->left;
        if (Z->right) Z->right->parent = Z;
        Y->left = Z;
        Z->parent = Y;
        Y->parent = X->parent;
        if (tree_is_left_child(X))
            X->parent->left = Y;
        else
            X->parent->right = Y;
        Y->right = X;
        X->parent = Y;
        return Y;
    }

    template<class base_tree_node_ptr_t>
    base_tree_node_ptr_t tree_root_right_left_rotate(base_tree_node_ptr_t X, base_tree_node_ptr_t Z)
    {
        base_tree_node_ptr_t Y = Z->left;
        X->right = Y->left;
        if (X->right) X->right->parent = X;
        Z->left = Y->right;
        if (Z->left) Z->left->parent = Z;
        Y->right = Z;
        Z->parent = Y;
        Y->parent = X->parent;
        Y->left = X;
        X->parent = Y;
        return Y;
    }

    template<class base_tree_node_ptr_t>
    base_tree_node_ptr_t tree_root_left_right_rotate(base_tree_node_ptr_t X, base_tree_node_ptr_t Z)
    {
        base_tree_node_ptr_t Y = Z->right;
        X->left = Y->right;
        if (X->left) X->left->parent = X;
        Z->right = Y->left;
        if (Z->right) Z->right->parent = Z;
        Y->left = Z;
        Z->parent = Y;
        Y->parent = X->parent;
        Y->right = X;
        X->parent = Y;
        return Y;
    }

}

//tree_node typedef
namespace BBST
{
    //indirect type passing with crtp https://stackoverflow.com/questions/6006614/c-static-polymorphism-crtp-and-using-typedefs-from-derived-classes
    template<typename derived_t>
    struct tree_node_base_traits;

    template<class tree_node_impl>
    struct base_tree_node
    {
        typedef typename tree_node_base_traits<tree_node_impl>::impl_type impl_type;

        base_tree_node *parent;
        tree_node_impl *left, *right;

        //left initialization for implementation class
        base_tree_node() = default;

        base_tree_node(base_tree_node *parent_, tree_node_impl *left_, tree_node_impl *right_)
                :
                parent(parent_)
                , left(left_)
                , right(right_)
        {}

        impl_type *parent_unsafe()
        {
            ASSERT(parent != nullptr && (parent->parent != nullptr || parent->right != nullptr), "fuck up unsafe cast");
            return static_cast<impl_type *>(parent);
        }

        impl_type *self_downcast_unsafe()
        {
            return static_cast<impl_type *>(this);
        }

        const impl_type *self_downcast_unsafe() const
        {
            return static_cast<const impl_type *>(this);
        }

        static inline base_tree_node *zero_initialize()
        {
            return new base_tree_node(nullptr, nullptr, nullptr);
        }
    };
}

//value_type
//TODO: compressed_pair for key and mapped (for set)
//TODO: https://stackoverflow.com/questions/31623423/why-does-libcs-implementation-of-map-use-this-union
namespace BBST
{
    template<class key_t, class mapped_t, class metadata_t>
    struct exposure
    {
        const key_t key;
        metadata_t metadata;
        mapped_t mapped;

        typedef const key_t key_type;
        typedef mapped_t mapped_type;
        typedef metadata_t metadata_type;

        template<class key_forward_t=key_t, class mapped_forward_t= mapped_t, class metadata_forward_t=metadata_t>
        requires (std::is_same_v<key_t, std::remove_reference_t<key_forward_t>> &&
                  std::is_same_v<mapped_t, std::remove_reference_t<mapped_forward_t>> &&
                  std::is_same_v<metadata_type, std::remove_reference_t<metadata_forward_t>>)
        explicit exposure(key_forward_t &&key_ = key_t(), metadata_forward_t &&metadata_ = metadata_t(), mapped_forward_t &&mapped_ = mapped_t())
                :
                key(std::forward<key_forward_t>(key_))
                , metadata(std::forward<mapped_forward_t>(metadata_))
                , mapped(std::forward<mapped_forward_t>(mapped_))
        {}
    };
}
//tree utils
namespace BBST
{
    template<class tree_node_base_ptr_t>
    tree_node_base_ptr_t tree_min(tree_node_base_ptr_t ptr)
    {
        ASSERT(ptr != nullptr, "ptr should not be null");
        while (ptr->left != nullptr) ptr = ptr->left;
        return ptr;
    }

    template<class tree_node_base_ptr_t>
    tree_node_base_ptr_t tree_max(tree_node_base_ptr_t ptr)
    {
        ASSERT(ptr != nullptr, "ptr should not be null");
        while (ptr->right != nullptr) ptr = ptr->right;
        return ptr;
    }

    template<class key_t, class base_tree_node_ptr_t, class comparator_t>
    requires (std::predicate<const comparator_t &, const key_t &, const key_t &> &&
              std::same_as<const key_t, typename std::remove_pointer_t<base_tree_node_ptr_t>::impl_type::key_type>)
    base_tree_node_ptr_t lower_bound(base_tree_node_ptr_t root_parent, const key_t &key, const comparator_t &comp)
    {

        base_tree_node_ptr_t result = root_parent;
        auto current = result->left;
        while (current != nullptr)
        {
            if (!comp(current->key(), key))
                result = std::exchange(current, current->left);
            else
                current = current->right;
        }
        return result;
    }

    template<class key_t, class base_tree_node_ptr_t, class comparator_t>
    requires (std::predicate<const comparator_t &, const key_t &, const key_t &> &&
              std::same_as<const key_t, typename std::remove_pointer_t<base_tree_node_ptr_t>::impl_type::key_type>)
    base_tree_node_ptr_t find(base_tree_node_ptr_t root_parent, const key_t &key, const comparator_t &comp)
    {
        base_tree_node_ptr_t p = lower_bound(root_parent, key, comp);
        if (p != root_parent && !comp(key, p->self_downcast_unsafe()->key()))
            return p;
        return root_parent;
    }

    template<class key_t, class base_tree_node_ptr_t, class impl_tree_node_ptr_t, class comparator_t>
    std::pair<impl_tree_node_ptr_t &, base_tree_node_ptr_t>
    find_equal_or_insert_pos(const key_t &key, base_tree_node_ptr_t end_node, const comparator_t &comp)
    requires (std::is_same_v<typename std::remove_pointer_t<base_tree_node_ptr_t>::impl_type, std::remove_pointer_t<impl_tree_node_ptr_t>> &&
              std::predicate<const comparator_t &, const key_t &, const key_t &>)
    {
        impl_tree_node_ptr_t *parent_link = &(end_node->left);
        impl_tree_node_ptr_t current_node_ptr = *parent_link;
        if (current_node_ptr != nullptr)
        {
            while (true)
            {
                if (comp(key, current_node_ptr->value_.key))
                {
                    if (current_node_ptr->left != nullptr)
                    {
                        parent_link = &(current_node_ptr->left);
                        current_node_ptr = current_node_ptr->left;
                    }
                    else
                    {
                        return {current_node_ptr->left, current_node_ptr};
                    }
                }
                else if (comp(current_node_ptr->value_.key, key))
                {
                    if (current_node_ptr->right != nullptr)
                    {
                        parent_link = &(current_node_ptr->right);
                        current_node_ptr = current_node_ptr->right;
                    }
                    else
                    {
                        return {current_node_ptr->right, current_node_ptr};
                    }
                }
                else
                {
                    return {*parent_link, current_node_ptr};
                }
            }
        }
        return {*parent_link, end_node};
    }

}

namespace BBST
{
    template<class T>
    T abs_difference(T a, T b)
    {
        return a < b ? b - a : a - b;
    }
}
#endif //BBST_TREE_UTILS_H
