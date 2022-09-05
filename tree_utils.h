#ifndef BBST_TREE_UTILS_H
#define BBST_TREE_UTILS_H

#include <cassert>
#include <utility>
#include <concepts>
#include <iostream>
//declaration
namespace BBST
{

#ifndef NDEBUG
#define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << (message) << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

    template<class base_tree_node_ptr_t>
    inline bool tree_is_left_child(base_tree_node_ptr_t ptr) noexcept;

    /*
     *   P              R
     *  / \            / \
     * L   R    ->    P   RR
     *    / \        / \
     *   RL  RR     L   RL
     *  It's the caller responsibility to update invariant on P and R (P->parent)
     */
    template<class base_tree_node_ptr_t>
    void unguarded_tree_left_rotate(base_tree_node_ptr_t P) noexcept;

    /*
     *      P              L
     *     / \            / \
     *    L   R    ->    LL  P
     *   / \                / \
     *  LL  LR             LR  R
     *  It's the caller responsibility to update invariant on P and L (P->parent)
     */
    template<class base_tree_node_ptr_t>
    void tree_right_rotate(base_tree_node_ptr_t P) noexcept;

    template<class base_tree_node_ptr_t>
    inline base_tree_node_ptr_t tree_prev_iter(base_tree_node_ptr_t ptr) noexcept;

    template<class base_tree_node_ptr_t>
    inline base_tree_node_ptr_t tree_next_iter(base_tree_node_ptr_t ptr) noexcept;

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

        static inline base_tree_node *zero_initialize()
        {
            return new base_tree_node(nullptr, nullptr, nullptr);
        }
    };

    template<class>
    class const_iterator_;

    //TODO: https://docs.microsoft.com/en-us/cpp/standard-library/sample-container-class?view=msvc-170
    template<class base_tree_node_t>
    class iterator_
    {
    public:
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef std::bidirectional_iterator_tag iterator_category;

    private:
        base_tree_node_ptr_t ptr;

        typedef typename base_tree_node_t::impl_type impl_type;

    public:
        typedef typename impl_type::value_type value_type;
        typedef const_iterator_<impl_type> const_iterator;
        typedef base_tree_node_ptr_t pointer;

        inline iterator_(base_tree_node_ptr_t ptr_ = nullptr) noexcept: ptr(ptr_)
        {}

        inline auto get() const noexcept
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

        inline iterator_ &operator++()
        {
            ptr = tree_next_iter(ptr);
            return *this;
        }

        inline iterator_ &operator--()
        {
            ptr = tree_prev_iter(ptr);
            return *this;
        }

        inline const_iterator operator++(int)
        {
            const_iterator temp(*this);
            ++(*this);
            return temp;
        }

        inline const_iterator operator--(int)
        {
            const_iterator temp(*this);
            --(*this);
            return temp;
        }

        friend inline bool operator==(const iterator_ &lhs, const iterator_ &rhs)
        {
            return lhs.ptr == rhs.ptr;
        }

        friend inline bool operator!=(const iterator_ &lhs, const iterator_ &rhs)
        {
            return lhs.ptr != rhs.ptr;
        }
    };

    template<class base_tree_node_t>
    class const_iterator_
    {
    public:
        typedef base_tree_node_t *base_tree_node_ptr_t;
        typedef std::bidirectional_iterator_tag iterator_category;

    private:
        base_tree_node_ptr_t ptr;

        typedef typename base_tree_node_t::impl_type impl_type;

    public:
        typedef typename impl_type::value_type value_type;
        typedef const_iterator_<impl_type> const_iterator;
        typedef base_tree_node_ptr_t pointer;

        explicit inline const_iterator_(base_tree_node_ptr_t ptr_ = nullptr) noexcept: ptr(ptr_)
        {}

        inline const_iterator_(iterator_<base_tree_node_t> o) : ptr(o.get())
        {};

        inline auto get() const noexcept
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

        inline const_iterator_ &operator++()
        {
            ptr = tree_next_iter(ptr);
            return *this;
        }

        inline const_iterator_ &operator--()
        {
            ptr = tree_prev_iter(ptr);
            return *this;
        }

        inline const_iterator_ operator++(int)
        {
            const_iterator_ temp(*this);
            ++(*this);
            return temp;
        }

        inline const_iterator_ operator--(int)
        {
            const_iterator_ temp(*this);
            --(*this);
            return temp;
        }

        friend inline bool operator==(const const_iterator_ &lhs, const const_iterator_ &rhs)
        {
            return lhs.ptr == rhs.ptr;
        }

        friend inline bool operator!=(const const_iterator_ &lhs, const const_iterator_ &rhs)
        {
            return lhs.ptr != rhs.ptr;
        }
    };

    template<class Key, class It, class Comp>
    requires (std::predicate<const Comp &, const Key &, const Key &> && std::same_as<Key, typename It::key_type>)
    It lower_bound(It root_parent, const Key &key, const Comp &comp)
    {

        auto result = root_parent.get();
        auto current = result->left;
        while (current != nullptr)
        {
            if (!comp(current->key(), key))
                result = std::exchange(current, current->left);
            else
                current = current->right;
        }
        return {result};
    }

    template<class Key, class It, class Comp>
    requires (std::predicate<const Comp &, const Key &, const Key &> && std::same_as<Key, typename It::key_type>)
    It find(It root_parent, const Key &key, const Comp &comp)
    {
        It p = lower_bound(root_parent, key, comp);
        if (p != root_parent && comp(key, p->key))
            return p;
        return root_parent;
    }
}


//implementation
namespace BBST
{
    template<class TreeNodePtr>
    bool tree_is_left_child(TreeNodePtr ptr) noexcept
    {
        return ptr == ptr->parent->left;
    }

    template<class base_tree_node_t>
    base_tree_node_t tree_prev_iter(base_tree_node_t ptr) noexcept
    {
        if (ptr->left != nullptr)
        {
            ptr = ptr->left;
            while (ptr->right != nullptr)
                ptr = ptr->right;
            return ptr;
        }
        while (BBST::tree_is_left_child(ptr))
            ptr = ptr->parent;
        return ptr->parent;
    }

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
    void tree_right_rotate(base_tree_node_ptr_t P) noexcept
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

}

#endif //BBST_TREE_UTILS_H
