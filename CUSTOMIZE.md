# Customize your own tree

## What can the metadata store?
Generally any aggregate information (`SUM,COUNT,AVG,MIN,MAX,RANGE`).
To be precise, any information that is computable knowing only value from the child nodes.

## Write your split function
The split function is highly tailored toward the specific problem and has many optimization opportunities. Thus, this library only comes with the most general form of a split function (split by key and split by statistic order). There are `xxx_tree_join` and `xxx_tree_join_by_x` helper functions that make implement your own split function easier.
Split function is implemented by specializing the ` friend struct xxx_tree_custom_invoker`{:.cpp}. The implementation needs to be tagged to avoid being shadowed by other specialization.
[Example]()

## Write your metadata updator function
