#ifndef BBST_TREE_CUSTOM_INVOKE_H
#define BBST_TREE_CUSTOM_INVOKE_H

#include <type_traits>

namespace bbst
{
    template<class updator_t, class impl_tree_node_pointer_t> concept is_order_statistic_metadata_updator =
    requires(const updator_t updator, impl_tree_node_pointer_t ptr) {
        { updator_t::template get_order_metadata<impl_tree_node_pointer_t>(ptr) } -> std::integral;
        updator.template operator()<impl_tree_node_pointer_t>(ptr)//->std::is_void
                ;
    };

    struct order_statistic_metadata_updator_impl
    {
        template<class impl_tree_node_ptr_t>
        requires(std::is_integral_v<typename std::remove_pointer_t<impl_tree_node_ptr_t>::metadata_type>)
        void operator()(impl_tree_node_ptr_t ptr) const
        {
            ptr->metadata() = 1 + order_statistic_metadata_updator_impl::get_order_metadata(ptr->left) +
                              order_statistic_metadata_updator_impl::get_order_metadata(ptr->right);
        }

        template<class impl_tree_node_ptr_t>
        requires(std::is_integral_v<typename std::remove_pointer_t<impl_tree_node_ptr_t>::metadata_type>)
        static inline typename std::remove_pointer_t<impl_tree_node_ptr_t>::metadata_type get_order_metadata(impl_tree_node_ptr_t p)
        {
            return p == nullptr ? 0 : p->metadata();
        };
    };
}

#endif //BBST_TREE_CUSTOM_INVOKE_H
