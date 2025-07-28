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

rm $build_exec
ln -s build/compiler $build_exec
