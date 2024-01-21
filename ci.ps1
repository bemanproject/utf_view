$ErrorActionPreference = "Stop"

Function Test {
    param (
        [string[]]$args
    )
    Remove-Item -Recurse -Force "${PSScriptRoot}\build"
    New-Item -Path "${PSScriptRoot}\build" -ItemType Directory -Force | Out-Null
    Set-Location "${PSScriptRoot}\build"
    cmake "${PSScriptRoot}" -DCMAKE_BUILD_TYPE=Debug -GNinja $args
    cmake --build "${PSScriptRoot}\build" -j $([Environment]::ProcessorCount)
    & "${PSScriptRoot}\build\test_p2728"
	if ($LASTEXITCODE -ne 0) {
		throw 'test failure'
	}
	Set-Location "${PSScriptRoot}"
}

Test
Test -DWCHAR_T_IS_CODE_UNIT=On
Test -DWCHAR_T_IS_CODE_UNIT=On -DCHAR_IS_CODE_UNIT=On
Test -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
Test -DWCHAR_T_IS_CODE_UNIT=On -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
Test -DWCHAR_T_IS_CODE_UNIT=On -DCHAR_IS_CODE_UNIT=On -DCHARN_T_POINTERS_ARE_RANGE_LIKE=On
