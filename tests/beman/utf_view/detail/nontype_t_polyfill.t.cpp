#include <beman/utf_view/detail/nontype_t_polyfill.hpp>
#include "../framework.hpp" 

TEST(NontypeTPolyfillTest, Instantiation) {
    auto n = beman::utf_view::detail::nontype<42>;
    
    (void)n; 

    EXPECT_TRUE(true);
}
