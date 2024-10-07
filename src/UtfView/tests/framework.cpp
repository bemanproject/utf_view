// SPDX-License-Identifier: GPL-3.0-only

#include <functional>
#include <map>
#include <string>

namespace utfview::tests {

namespace framework {

  std::map<std::string, std::function<bool()>>& tests() {
    static std::map<std::string, std::function<bool()>> result{};
    return result;
  }

} // namespace framework

} // namespace utfview::tests
