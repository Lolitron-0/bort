#!/bin/bash

set -eux

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'
root=$(dirname $0)/../
build_dir=$1
corpus_dir=$root/tests/corpus

for file in $corpus_dir/*.c; do
  $build_dir/bort --dump-ast --emit-ir $file 

  if [ "$?" -eq "0" ]; then
    echo -e "${GREEN}OK $file${NC}\n"
  else
    echo -e "${RED}FAIL $file${NC}\n"
  fi
done


