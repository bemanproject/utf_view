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

    pattern = r'\bexposition_only_[a-zA-Z0-9_]+\b'
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
# body is wrapped in a pair of comments:
#
#   /* EFFECTS */
#   constexpr void exposition_only_f() { // @*exposition only*@
#     g();
#   }
#   /* !EFFECTS */
#
# Its declaration goes into the synopsis in place of its definition, and it
# gets a block after the synopsis, in the order such functions appear.
def split_function(lines):
    # The declaration is everything up to the brace that opens the body, which
    # is at the end of a line, possibly followed by a comment.
    for i, line in enumerate(lines):
        code, has_comment, comment = line.rstrip("\n").partition("//")
        code = code.rstrip()
        if code.endswith("{"):
            break
    declaration = lines[:i]
    code = code[:-1].rstrip()
    if code:
        declaration.append(code + ";" + (" //" + comment if has_comment else "") + "\n")
    else:
        declaration[-1] = declaration[-1].rstrip("\n") + ";\n"
    # The body ends before the closing brace, which is the last line.
    body = lines[i + 1:]
    while not body[-1].strip():
        body.pop()
    body.pop()
    return declaration, body


# A member is marked as exposition-only where it is declared in the synopsis,
# and not again where it is described.
def remove_exposition_only_comments(lines):
    return [re.sub(r'[ \t]*// @\*exposition only\*@', '', line) for line in lines]


def format_effects(declaration, body):
    declaration = remove_exposition_only_comments(declaration)
    return [
        "\n```cpp\n",
        textwrap.dedent("".join(declaration)),
        "```\n\n",
        "_Effects_: Equivalent to:\n\n",
        "```cpp\n",
        textwrap.dedent("".join(body)),
        "```\n",
    ]


def process_file(file_path):
    in_paper_section = False
    result_lines = []
    effects_lines = []
    effects_function = None

    with open(file_path, 'r') as file:
        for line in file:
            line = convert_snake_to_kebab(line);
            if "/* PAPER: " in line:
                result_lines.append(line[line.find("/* PAPER:") + len("/* PAPER: "):-4])
                result_lines.append("\n")
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

            if "/* !EFFECTS */" in line:
                declaration, body = split_function(effects_function)
                if in_paper_section:
                    result_lines += declaration
                effects_lines += format_effects(declaration, body)
                effects_function = None
                continue
            elif "/* EFFECTS */" in line:
                effects_function = []
                continue

            if effects_function is not None:
                effects_function.append(line)
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
