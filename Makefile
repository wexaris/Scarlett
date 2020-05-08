# This Makefile is just a shorthand for
# running cmake in debug or release mode

ROOT_DIR := $(shell dirname $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_TYPE ?= Debug
BUILD_DIR := $(shell echo $(BUILD_TYPE) | tr A-Z a-z)

THREAD_NUM ?= 4

.PHONY: all build debug release

all: release

.ONESHELL:
build:
	mkdir -p build/$(BUILD_DIR) && cd build/$(BUILD_DIR)
	cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(ROOT_DIR)
	cmake --build . --target scar --config $(BUILD_TYPE) -- -j$(THREAD_NUM)

debug:
	$(MAKE) build BUILD_TYPE=Debug

release:
	$(MAKE) build BUILD_TYPE=Release

clean:
	rm -rf build/debug bin/debug
	rm -rf build/release bin/release