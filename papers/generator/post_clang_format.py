#!/usr/bin/env python3

# SPDX-License-Identifier: BSL-1.0

import re
import sys
import textwrap

def convert_snake_to_kebab(text):
    def snake_to_kebab(match):
        word = match.group(0)
        word = word.replace('exposition_only_', '', 1)
        if word[-1] == "_":
            kebab_case_word = word
        else:
            kebab_case_word = word.replace('_', '-')
        return f"@*{kebab_case_word}*@"

    pattern = r'\bexposition_only_[a-zA-Z_]+\b'
    result = re.sub(pattern, snake_to_kebab, text)
    return result


# The draft spells calls to the range access customization point objects and
# the range iterator operations as `ranges::begin(x)`, etc., while types and
# concepts from std::ranges are unqualified. Inside a view class an unqualified
# `begin(x)` would also find the member function.
RANGES_CALLABLES = (
    "begin", "end", "cbegin", "cend", "rbegin", "rend", "crbegin", "crend",
    "size", "ssize", "reserve_hint", "empty", "data", "cdata",
    "advance", "distance", "next", "prev",
    "iter_move", "iter_swap", "swap",
)

RANGES_CALL_PATTERN = re.compile(
    r'\bstd::ranges::(' + '|'.join(RANGES_CALLABLES) + r')(?=\s*\()')


def keep_ranges_qualifier_on_calls(text):
    return RANGES_CALL_PATTERN.sub(r'ranges::\1', text)


# A function whose specification is "Effects: Equivalent to:" followed by its
# body is marked up like this, outside of a PAPER section:
#
#   constexpr void exposition_only_f() {
#     /* EFFECTS:   constexpr void @*f*@(); // @*exposition only*@ */
#     g();
#     /* !EFFECTS */
#   }
#
# The declaration in the EFFECTS comment goes into the synopsis, like a
# "PAPER:" line. The lines up to the !EFFECTS comment are the body. Each such
# function gets a block after the synopsis, in the order they appear.
def format_effects(declaration, body_lines):
    return [
        "\n```cpp\n",
        declaration.strip() + "\n",
        "```\n\n",
        "_Effects_: Equivalent to:\n\n",
        "```cpp\n",
        textwrap.dedent("".join(body_lines)),
        "```\n",
    ]


def process_file(file_path):
    in_paper_section = False
    result_lines = []
    effects_lines = []
    effects_declaration = None
    effects_body = []

    with open(file_path, 'r') as file:
        for line in file:
            line = convert_snake_to_kebab(line);
            if "/* PAPER: " in line:
                result_lines.append(line[line.find("/* PAPER:") + len("/* PAPER: "):-4])
                result_lines.append("\n")
                continue
            if "/* EFFECTS: " in line:
                effects_declaration = line[line.find("/* EFFECTS:") + len("/* EFFECTS: "):-4]
                result_lines.append(effects_declaration)
                result_lines.append("\n")
                continue
            if "/* !EFFECTS */" in line:
                effects_lines += format_effects(effects_declaration, effects_body)
                effects_declaration = None
                effects_body = []
                continue

            line = keep_ranges_qualifier_on_calls(line)
            line = line.replace("std::ranges::", "")
            line = line.replace("std::", "")
            line = line.replace("detail::", "")
            line = line.replace("boost::stl_interfaces::", "")
            line = line.replace("CONSTEXPR_UNLESS_MSVC", "constexpr")
            line = re.sub(r'\bmove\(', 'std::move(', line)

            if "/* !PAPER */" in line:
                in_paper_section = False
                continue
            elif "/* PAPER */" in line:
                in_paper_section = True
                continue

            if effects_declaration is not None:
                effects_body.append(line)
            elif in_paper_section:
                result_lines.append(line)

    return result_lines + effects_lines

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python post_clang_format.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]

    processed_lines = process_file(file_path)

    for line in processed_lines:
        print(line, end="")
