#/usr/bin/env bash

set -euo pipefail

set -x

declare generator_script_dir=$(dirname $BASH_SOURCE)

function process_file() {
    local file="$1"; shift
    local build_dir="$1" ; shift
    cp $generator_script_dir/../../$file $build_dir
    local file_copy=$build_dir/${file##*/}
    clang-format-20 -i -style=file $file_copy
    $generator_script_dir/post_clang_format.py $file_copy > ${file_copy}.md
    rm $file_copy
}

function main() {
    local files=(
        include/beman/utf_view/code_unit_view.hpp
        include/beman/utf_view/detail/concepts.hpp
        include/beman/utf_view/null_term.hpp
        include/beman/utf_view/to_utf_view.hpp)
    local build_dir=$generator_script_dir/build
    mkdir $build_dir
    cp $generator_script_dir/.clang-format $build_dir
    for file in ${files[@]} ; do
        process_file "$file" "$build_dir"
    done
}

main "$@"
