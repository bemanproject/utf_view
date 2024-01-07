#!/usr/bin/env bash

set -euo pipefail

declare ci_script_dir=$(realpath $(dirname "$BASH_SOURCE"))

function test() {
    local -a args=("$@")
    rm -rf "${ci_script_dir}/build"
    mkdir -p "${ci_script_dir}/build"
    cd "${ci_script_dir}/build"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -GNinja "${args[@]}"
    cmake --build . -j$(nproc)
    "${ci_script_dir}/build/p2728_test"
}

test
test -DUSE_FORMAT_ENUMERATION=On
test -DUSE_FORMAT_ENUMERATION=On -DWCHAR_T_IS_CODE_UNIT=On
test -DUSE_FORMAT_ENUMERATION=On -DWCHAR_T_IS_CODE_UNIT=On -DCHAR_IS_CODE_UNIT=On
test -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
test -DUSE_FORMAT_ENUMERATION=On -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
test -DUSE_FORMAT_ENUMERATION=On -DWCHAR_T_IS_CODE_UNIT=On -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
test -DUSE_FORMAT_ENUMERATION=On -DWCHAR_T_IS_CODE_UNIT=On -DCHAR_IS_CODE_UNIT=On -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
