#!/usr/bin/env python3

import re
import sys

def convert_snake_to_kebab(text):
    def snake_to_kebab(match):
        word = match.group(0)
        word = word.replace('exposition_only_', '', 1)
        kebab_case_word = word.replace('_', '-')
        return f"@*{kebab_case_word}*@"

    pattern = r'\bexposition_only_[a-zA-Z_]+\b'
    result = re.sub(pattern, snake_to_kebab, text)
    return result


def process_file(file_path):
    in_paper_section = False
    result_lines = []

    with open(file_path, 'r') as file:
        for line in file:
            line = convert_snake_to_kebab(line);
            if "/* PAPER: " in line:
                result_lines.append(line[line.find("/* PAPER:") + len("/* PAPER: "):-4])
                result_lines.append("\n")
                continue

            line = line.replace("std::ranges::", "")
            line = line.replace("std::", "")
            line = line.replace("detail::", "")
            line = line.replace("boost::stl_interfaces::", "")
            line = line.replace("CONSTEXPR_UNLESS_MSVC", "constexpr")

            if "/* !PAPER */" in line:
                in_paper_section = False
                continue
            elif "/* PAPER */" in line:
                in_paper_section = True
                continue

            if in_paper_section:
                result_lines.append(line)

    return result_lines

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python post_clang_format.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]

    processed_lines = process_file(file_path)

    for line in processed_lines:
        print(line, end="")
