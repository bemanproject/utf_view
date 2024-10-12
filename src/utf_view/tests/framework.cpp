// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <functional>
#include <map>
#include <string>

namespace utf_view::tests {

namespace framework {

  std::map<std::string, std::function<bool()>>& tests() {
    static std::map<std::string, std::function<bool()>> result{};
    return result;
  }

} // namespace framework

} // namespace utf_view::tests
