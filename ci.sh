#!/usr/bin/env bash

set -euo pipefail

function main() {
    local -a cxx_flags=("$@")
    local checkout="$PWD"
    mkdir "$checkout/build"
    cd "$checkout/build"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=23 -DBEMAN_UTF_VIEW_BUILD_TESTING=On "$@"
    cmake --build . --parallel
    export ASAN_OPTIONS=alloc_dealloc_mismatch=0 # https://github.com/llvm/llvm-project/issues/59432
    ctest -C Debug
}

main "$@"
