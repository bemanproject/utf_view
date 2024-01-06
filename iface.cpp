module;

#include <iostream>

export module p2728;

import null_term;

namespace p2728 {

export void foo() {
  for (char c : null_term::null_term("Hello, world!\n")) {
    std::cout << c;
  }
}

} // namespace p2728
