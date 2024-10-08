// SPDX-License-Identifier: BSL-1.0

//   Copyright Eddie Nolan and Jonathan Wakely 2023 - 2024.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <iostream>
#include <tests/framework.hpp>

int main() {
  for (auto& [test_name, test] : utfview::tests::framework::tests()) {
    if (!test()) {
      std::cerr << test_name << " failed";
      return EXIT_FAILURE;
    }
  }
}
