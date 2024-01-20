#!/usr/bin/env python3

import argparse
import os
import re

def kebab_to_snake(text):
    """Convert kebab-case to snake_case."""
    return text.replace('-', '_')

def process_file(input_path, output_path):
    """Process the file: read, convert, and write."""
    with open(input_path, 'r') as file:
        content = file.read()

    # Replace text between @* *@ from kebab-case to snake_case
    content = re.sub(r'@\*(.*?)\*@', lambda m: kebab_to_snake(m.group(1)), content)

    with open(output_path, 'w') as file:
        file.write(content)

def main():
    parser = argparse.ArgumentParser(description='Process .cpp.in files.')
    parser.add_argument('input_files', nargs='+', help='Input .cpp.in files')
    parser.add_argument('--out-dir', required=True, help='Output directory')

    args = parser.parse_args()

    for input_path in args.input_files:
        if not input_path.endswith('.cpp.in'):
            raise ValueError(f"File {input_path} does not have the correct extension (.cpp.in)")

        base_name = os.path.basename(input_path)
        output_file = base_name.replace('.cpp.in', '.cpp')
        output_path = os.path.join(args.out_dir, output_file)

        process_file(input_path, output_path)
        print(f"Processed {input_path} -> {output_path}")

if __name__ == "__main__":
    main()
