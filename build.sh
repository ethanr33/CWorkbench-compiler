#!/bin/bash

if ! [ -d build ]; then
  mkdir build
fi

export PATH="$PWD:$PATH"

i=1;
build_debug=0
while [[ i -le $# ]]; do
    case ${!i} in
            --debug|-d) build_debug=1 ;;
        *) echo "Unknown option: ${!i}" ; exit 1 ;;
    esac
    (( i++ ))
done 

cd build

if ! (($build_debug)); then
  cmake ..

  if ! (($? == 0)); then
    exit 1
  fi
else 
  cmake .. -DCMAKE_BUILD_TYPE=Debug

  if ! (($? == 0)); then
    exit 1
  fi
fi

make
cd ..

build_exec=compiler

if ! [ -f $build_exec ]; then
  ln -s build/compiler $build_exec
fi