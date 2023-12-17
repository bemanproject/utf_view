// SPDX-License-Identifier: GPL-3.0-only

#ifndef UTFVIEW_DETAIL_ERRONEOUS_BEHAVIOR_GLOBAL_HPP
#define UTFVIEW_DETAIL_ERRONEOUS_BEHAVIOR_GLOBAL_HPP

namespace utfview::detail {

inline bool& erroneous_behavior_global() {
  static bool global{};
  return global;
}

} // namespace utfview::detail

#endif // UTFVIEW_DETAIL_ERRONEOUS_BEHAVIOR_GLOBAL_HPP
