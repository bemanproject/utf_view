#!/usr/bin/env python3

import argparse
import shutil

def main():
    # Create the parser
    parser = argparse.ArgumentParser(description='Copy a file to a new location.')

    # Add the positional argument for the source file
    parser.add_argument('source', type=str, help='The source file path')

    # Add the named parameter '-o' for the destination file
    parser.add_argument('-o', '--output', type=str, help='The destination file path', required=True)

    # Parse the arguments
    args = parser.parse_args()

    # Copy the file
    shutil.copy(args.source, args.output)
    print(f"File copied from {args.source} to {args.output}")

if __name__ == "__main__":
    main()
