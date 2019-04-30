# This Makefile is just a shorthand for
# running cmake in debug or release mode

ROOT_DIR := $(shell dirname $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_TYPE ?= Release
BUILD_DIR := $(shell echo $(BUILD_TYPE) | tr A-Z a-z)


# Execute everything in one command shell
.ONESHELL:

.PHONY: all build rebuild debug release relwithdebinfo minsizerel clean_build clean


all: release

rebuild: clean_build build
build:
	mkdir -p build/$(BUILD_DIR) && cd build/$(BUILD_DIR)
	cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(ROOT_DIR)
	echo && make

debug:
	$(MAKE) build BUILD_TYPE=Debug

release:
	$(MAKE) build BUILD_TYPE=Release

relwithdebinfo:
	$(MAKE) build BUILD_TYPE=RelWithDebInfo

minsizerel:
	$(MAKE) build BUILD_TYPE=MinSizeRel

clean_build:
	rm -rf build/$(BUILD_DIR) bin/$(BUILD_DIR)

clean:
	rm -rf build/debug bin/debug
	rm -rf build/release bin/release
	rm -rf build/minsizerel bin/minsizerel
	rm -rf build/relwithdebinfo bin/relwithdebinfo