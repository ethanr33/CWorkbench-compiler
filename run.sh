#!/bin/bash

if ! [ -d tmp ]; then
  mkdir -p tmp
fi

build_exec=compiler
if [ -f $1 ]; then
    ./$build_exec $1

    if [ $? == 0 ]; then
        nasm tmp/out.asm -f elf64 -o tmp/out.o
        ld tmp/out.o -o tmp/out
        tmp/out
        exit $?
    else
        echo "Compiler exited with error code $?"
    fi

else
    echo "$1: file not found"
fi