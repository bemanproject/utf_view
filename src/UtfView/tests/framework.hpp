// SPDX-License-Identifier: GPL-3.0-only

#ifndef UTFVIEW_TESTS_FRAMEWORK_HPP
#define UTFVIEW_TESTS_FRAMEWORK_HPP

#include <functional>
#include <map>
#include <string>

namespace utfview::tests {

namespace framework {

  std::map<std::string, std::function<bool()>>& tests();

} // namespace framework

} // namespace utfview::tests

#endif // UTFVIEW_TESTS_FRAMEWORK_HPP
