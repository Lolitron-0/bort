#!/bin/bash

set -eu

mkdir -p build 

cmake -S . \
			-B build \
			-G Ninja \
			-DCMAKE_CXX_COMPILER=clang++ \
			-DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined,leak" \
			-DCMAKE_BUILD_TYPE=Debug \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=1 
cmake --build build --parallel $(nproc)
cp -f build/compile_commands.json .

if [ $# -ne 0 ] && [ "$1" == "run" ]; then
	echo -e "----------------------------------\n"
  set -eux
	./build/bort -E ./tests/corpus/basic.c
  set +x
  set -eu
fi
