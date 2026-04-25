#include <beman/utf_view/detail/constexpr_unless_msvc.hpp>
#include "../framework.hpp"

TEST(ConstexprUnlessMsvcTest, CompilationCheck) {
    EXPECT_TRUE(true);
}