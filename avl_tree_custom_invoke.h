#ifndef BBST_AVL_TREE_CUSTOM_INVOKE_H
#define BBST_AVL_TREE_CUSTOM_INVOKE_H

namespace bbst
{
    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t, class tag>
    struct avl_tree_custom_invoke {};

    struct avl_tree_custom_invoke_default_tag {};

    template<class key_t, class mapped_t, class metadata_t, class metadata_updator_t, class comparator_t>
    struct avl_tree_custom_invoke<key_t, mapped_t, metadata_t, metadata_updator_t, comparator_t, avl_tree_custom_invoke_default_tag>
    {
        using avl_tree_t = avl_tree<key_t, mapped_t, metadata_t, metadata_updator_t, comparator_t>;
        using avl_tree_header_t = avl_tree_header<key_t, mapped_t, metadata_t>;
        using avl_tree_node_ptr_t = typename avl_tree_t::avl_tree_node_ptr_t;

        static inline avl_tree_header_t to_avl_tree_header(avl_tree_t &&tree)
        {
            avl_tree_node_ptr_t root = std::exchange(tree.end_node_.left, nullptr);
            tree.begin_node_ = &tree.end_node_;
            uint32_t height = std::exchange(tree.height_, 1);
            return {root, height};
        };

        template<bool equal_on_left_side>
        static std::pair<avl_tree_t, avl_tree_t> split_by_key(avl_tree_t &&tree, const key_t &key)
        {
            auto &comparator = tree.comp_;
            auto &metadata_updator = tree.updator_;
            auto split_routine = [&comparator, &metadata_updator](auto self, avl_tree_header_t header
                                                                  , const key_t &key) -> std::pair<avl_tree_header_t, avl_tree_header_t>
            {
                if (header.empty())
                {
                    return {avl_tree_header_t::empty_header(), avl_tree_header_t::empty_header()};
                }
                if (comparator(key, header.root_->key()))
                {
                    //key < header.root_
                    auto [left_header, right_header] = self(self, avl_tree_header_t(header.root_->left,
                            header.height_ - (header.root_->height_diff_ > 0 ? 2 : 1)), key);
                    return {left_header, bbst::avl_tree_join_x(right_header, header.root_, avl_tree_header_t(header.root_->right,
                            header.height_ - (header.root_->height_diff_ < 0 ? 2 : 1)), metadata_updator, comparator)};
                }
                else
                {
                    if (comparator(header.root_->key(), key))
                    {
                        //header.root_ < key
                        auto [left_header, right_header] = self(self, avl_tree_header_t(header.root_->right,
                                header.height_ - (header.root_->height_diff_ < 0 ? 2 : 1)), key);
                        return {bbst::avl_tree_join_x(avl_tree_header_t(header.root_->left, header.height_ - (header.root_->height_diff_ > 0 ? 2 : 1)), header
                                .root_, left_header, metadata_updator, comparator), right_header};
                    }
                    else
                    {
                        //==
                        int32_t root_diff = header.root_->height_diff_;
                        uint32_t root_height = header.height_;
                        avl_tree_node_ptr_t left = header.root_->left;
                        avl_tree_node_ptr_t right = header.root_->right;
                        if constexpr(equal_on_left_side)
                        {
                            return {bbst::avl_tree_join_x(avl_tree_header_t(left, root_height - (root_diff > 0 ? 2 : 1)), header
                                    .root_, avl_tree_header_t::empty_header(), metadata_updator, comparator),
                                    avl_tree_header_t(right, root_height - (root_diff < 0 ? 2 : 1))};
                        }
                        else
                        {
                            return {avl_tree_header_t(left, root_height - (root_diff > 0 ? 2 : 1)),
                                    bbst::avl_tree_join_x(avl_tree_header_t::empty_header(), header.root_, avl_tree_header_t(right,
                                            root_height - (root_diff < 0 ? 2 : 1)), metadata_updator, comparator)};
                        }
                    }
                }
            };
            avl_tree_header_t header = to_avl_tree_header(std::move(tree));
            ASSERT(avl_tree_header_invariant(header), "pre condition failed");
            auto [l, r] = split_routine(split_routine, header, key);
            ASSERT(avl_tree_header_invariant(l), "post condition failed");
            ASSERT(avl_tree_header_invariant(r), "post condition failed");
            return {avl_tree_t(l, metadata_updator, comparator), avl_tree_t(r, metadata_updator, comparator)};
        }
    };
}
#endif //BBST_AVL_TREE_CUSTOM_INVOKE_H
