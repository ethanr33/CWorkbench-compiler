#!/bin/bash


if ! [ -d build ]; then
  mkdir build
fi

export PATH="$PWD:$PATH"

cd build
cmake ..
make
cd ..

build_exec=compiler

if ! [ -f $build_exec ]; then
  ln -s build/compiler $build_exec
fi