#include <beman/utf_view/detail/fake_inplace_vector.hpp>
#include "../framework.hpp"

using beman::utf_view::detail::fake_inplace_vector;

TEST(FakeInplaceVectorTest, BasicOperations) {
    fake_inplace_vector<int, 5> vec;
    EXPECT_EQ(vec.size(), 0u);
    
    vec.push_back(42);
    EXPECT_EQ(vec.size(), 1u);
    EXPECT_EQ(vec[0], 42);

    fake_inplace_vector<int, 3> vec2{1, 2, 3};
    EXPECT_EQ(vec2.size(), 3u);
    EXPECT_EQ(vec2[2], 3);

    vec2.clear();
    EXPECT_EQ(vec2.size(), 0u);
}
