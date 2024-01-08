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
    "${ci_script_dir}/build/test_p2728"
}

test
test -DWCHAR_T_IS_CODE_UNIT=On
test -DWCHAR_T_IS_CODE_UNIT=On -DCHAR_IS_CODE_UNIT=On
test -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
test -DWCHAR_T_IS_CODE_UNIT=On -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
test -DWCHAR_T_IS_CODE_UNIT=On -DCHAR_IS_CODE_UNIT=On -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
