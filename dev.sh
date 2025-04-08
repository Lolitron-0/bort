#!/bin/bash

set -eu

mkdir -p build 

HI='\033[0;32m'
NC='\033[0m'

function build {
  cc=$1
  cxx=$2
  build_type=$3

  echo -e "${HI}Building $cc $build_type${NC}"

  cmake -S . \
        -B build/build-${cc}-${build_type} \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=$build_type  \
        -DCMAKE_C_COMPILER=$cc \
        -DCMAKE_CXX_COMPILER=$cxx 
  cmake --build build/build-${cc}-${build_type} --parallel $(nproc)
}


if [ $# -ne 0 ] && [ "$1" == "check-compile" ]; then
  build gcc g++ Release
  build gcc g++ Debug
  build clang clang++ Release
  build clang clang++ Debug
  exit 0
fi

cmake -S . \
			-B build \
			-G Ninja \
      -DBORT_BUILD_TESTS=ON \
			-DCMAKE_CXX_COMPILER=clang++ \
			-DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -Wall -Wextra -pedantic" \
			-DCMAKE_BUILD_TYPE=Debug \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=1 
cmake --build build --parallel $(nproc)
cp -f build/compile_commands.json .

if [ $# -ne 0 ] && [ "$1" == "run" ]; then
	echo -e "----------------------------------\n"
  set -eux
	./build/bort --dump-ast --emit-ir --dump-codegen-info -o - ./tests/corpus/loops.c
fi
