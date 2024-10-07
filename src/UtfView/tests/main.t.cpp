// SPDX-License-Identifier: GPL-3.0-only

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
