CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -Iinclude
LDFLAGS  ?=

SRC  := src/RubiksCube.cpp src/RubiksCube3dArray.cpp src/RubiksCube1dArray.cpp \
        src/RubiksCubeBitboard.cpp src/CornerPatternDatabase.cpp src/CornerDBMaker.cpp
OBJ  := $(SRC:src/%.cpp=build/%.o)
HDRS := $(wildcard include/*.h include/*/*.h)

all: rubiks run_tests

build/%.o: src/%.cpp $(HDRS) | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build data

rubiks: $(OBJ) build/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run_tests: $(OBJ) tests/test_main.cpp $(HDRS) | build
	$(CXX) $(CXXFLAGS) tests/test_main.cpp $(OBJ) -o $@ $(LDFLAGS)

test: run_tests
	./run_tests

test-full: run_tests
	./run_tests --full

clean:
	rm -rf build rubiks run_tests

.PHONY: all test test-full clean
