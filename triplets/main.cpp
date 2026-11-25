// SPDX-License-Identifier: BSL-1.0

import std;
import beman.utf_view;

auto main() -> int
{
    constexpr std::u8string_view input = u8"A\U0001F642";
    constexpr std::u16string_view expected = u"A\U0001F642";

    return std::ranges::equal(input | beman::utf_view::to_utf16, expected) ? 0 : 1;
}
