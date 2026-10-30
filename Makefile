CXX ?= g++
GO ?= $(if $(wildcard /workspace/.tools/go/bin/go),/workspace/.tools/go/bin/go,go)
PYTHON ?= python3
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
export GOCACHE := $(abspath build/go-cache)
export GOTOOLCHAIN := local
export GOPROXY := off
CPP_SOURCES := $(wildcard c++/*.cpp)
CPP_HEADERS := $(wildcard c++/*.h)
GO_SOURCES := $(filter-out %_test.go,$(wildcard go/*.go))

.PHONY: all test test-cpp test-go test-cli clean
all: build/dbms-cpp build/dbms-go

build:
	mkdir -p build

build/dbms-cpp: $(CPP_SOURCES) $(CPP_HEADERS) | build
	$(CXX) $(CXXFLAGS) $(CPP_SOURCES) -o $@

build/dbms-go: $(GO_SOURCES) go/go.mod | build
	cd go && $(GO) build -trimpath -o ../$@ .

build/test-cpp: tests/cpp_test.cpp $(filter-out c++/main.cpp,$(CPP_SOURCES)) $(CPP_HEADERS) | build
	$(CXX) -std=c++17 -O1 -g -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I c++ tests/cpp_test.cpp $(filter-out c++/main.cpp,$(CPP_SOURCES)) -o $@

test-cpp: build/test-cpp
	ASAN_OPTIONS=detect_leaks=1 ./build/test-cpp

test-go:
	cd go && $(GO) test -count=1 -v ./...
	cd go && $(GO) vet ./...

test-cli: all
	$(PYTHON) -m unittest discover -s tests -p 'test_*.py' -v

test: test-cpp test-go test-cli

clean:
	rm -rf build
