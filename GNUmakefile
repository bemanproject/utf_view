# GNUmakefile
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

MAKEFLAGS+= --no-builtin-rules          # Disable the built-in implicit rules.
MAKEFLAGS+= --no-builtin-variables      # Disable the built-in variable settings.
MAKEFLAGS+= --warn-undefined-variables  # Warn when an undefined variable is referenced.
.SUFFIXES:                              # Disable all suffix rules.
##################################################

ifeq ($(origin CXX),default)
  export CXX:= g++-16
  export CC:= gcc-16
endif

PRESET?=gcc-release
IMAGE?=ghcr.io/bemanproject/infra-containers-gcc:trunk

_build_path:=build/$(PRESET)

.PHONY: all check distclean format dockerbuild

# default target rule
all: .init compile_commands.json ## Make all with cmake workflow preset
	cmake --workflow --preset $(PRESET)

.init: CMakeUserPresets.json CMakePresets.json CMakeLists.txt ## Configure cmake preset in VERBOSE mode
	cmake --version
	cmake --preset $(PRESET) -D BEMAN_EXEMPLAR_USE_MODULES=ON \
		-D CMAKE_CXX_STANDARD=26 \
		-D BEMAN_UTF_VIEW_USE_MODULES=ON \
		-D CMAKE_PROJECT_TOP_LEVEL_INCLUDES=infra/cmake/use-fetch-content.cmake \
		--fresh --log-level=VERBOSE
	touch $@

check: .init compile_commands.json ## Run clang-tidy on examples
	run-clang-tidy examples

.PHONY: compile_commands.json
compile_commands.json: $(_build_path)/compile_commands.json
	if [ "X$(shell readlink compile_commands.json)" != "X$(_build_path)/compile_commands.json" ] ; then \
		ln -fs $< $@; \
	fi

CMakeUserPresets.json:: cmake/CMakeUserPresets.json
	-ln -fs $< $@

distclean: ## Remove all build artifacts
	rm -rf build .cache
	rm -f CMakeUserPresets.json \
	compile_commands.json \
	.init
	find . -name '*~' -delete

format: distclean  ## Format all files with pre-commit
	git ls-files ':(top,exclude)infra/cmake/' ::cmake.in ::*.cmake ::*CMakeLists.txt | xargs gersemi -i
	# -pre-commit autoupdate --freeze
	# pre-commit run --all

dockerbuild: ## Start docker image interactive
	docker run -it -v $(CURDIR):/src $(IMAGE)

vcpkg: ## Check the vcpkg config usage
	${HOME}/vcpkg/bootstrap-vcpkg.sh
	cmake -S triplets -B build/vcpkg-modules -G Ninja \
    -D CMAKE_CXX_STANDARD=26 \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
    -D VCPKG_OVERLAY_TRIPLETS="$(CURDIR)/triplets" \
    -D VCPKG_TARGET_TRIPLET=x64-osx-beman-modules
	cmake --build build/vcpkg-modules
	ctest --test-dir build/vcpkg-modules --output-on-failure

# Helper targets
.PHONY: env info

env: ## Show env
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

info: ## Show this help.
	@awk 'BEGIN {FS = ":.*?## "} /^[.a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST) | sort

# NOTE: double-colon targets which has no prerequisites must always remade by gmake?
#  Prerequisite 'GNUmakefile' is newer than target '.init'.
#  -> Must remake target '.init'.
# NOTE: impizit handled by gmake! GNUmakefile :: ;
cmake/CMakeUserPresets.json :: ;

# Anything we don't know how to build will use this rule.
% ::
	ninja -C $(_build_path) $(@)
