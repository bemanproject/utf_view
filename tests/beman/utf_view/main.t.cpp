// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan 2023 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <beman/utf_view/config.hpp>
#include <framework.hpp>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <iostream>
#endif

int main() {
  for (auto& [test_name, test] : beman::utf_view::tests::framework::tests()) {
    if (!test()) {
      std::cerr << test_name << " failed";
      return 1;
    }
  }
}
