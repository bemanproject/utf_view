#!/usr/bin/env python3

import argparse
import os
import re

def kebab_to_snake(text):
    """Convert kebab-case to snake_case."""
    return "exposition_only_" + text.replace('-', '_')

def process_file(input_path, output_path):
    """Process the file: read, convert, and write."""
    with open(input_path, 'r') as file:
        content = file.read()

    def snake_to_kebab(match):
        # Extract the matched text, excluding 'EO' and 'OE'
        text = match.group(1)
        # Replace underscores with hyphens to convert to kebab case
        kebab_case_text = text.replace('_', '-')
        # Return the converted text with 'EO' and 'OE'
        return f'@*{kebab_case_text}*@'

    # Regular expression to match 'EO...OE' with the text in between
    pattern = r'EO(.*?)OE'

    # Replace each match using the snake_to_kebab function
    result = re.sub(pattern, snake_to_kebab, content)

    with open(output_path, 'w') as file:
        file.write(result)

def main():
    parser = argparse.ArgumentParser(description='Process .cpp files.')
    parser.add_argument('input_files', nargs='+', help='Input .cpp files')
    parser.add_argument('--out-dir', required=True, help='Output directory')

    args = parser.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)

    for input_path in args.input_files:
        if not input_path.endswith('.cpp'):
            raise ValueError(f"File {input_path} does not have the correct extension (.cpp)")

        base_name = os.path.basename(input_path)
        output_file = base_name.replace('.cpp', '.cpp.md')
        output_path = os.path.join(args.out_dir, output_file)

        process_file(input_path, output_path)
        print(f"Processed {input_path} -> {output_path}")

if __name__ == "__main__":
    main()
