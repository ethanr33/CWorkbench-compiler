#!/bin/bash

if ! [ -d tmp ]; then
  mkdir -p tmp
fi

if [ $# -le 0 ]; then
    echo "Usage: ./run.sh <cpp file>" ;
    exit 1
fi 

i=2;
compile_only=0
while [[ i -le $# ]]; do
    case ${!i} in
            --compile-only|-c) compile_only=1 ;;
        *) echo "Unknown option: ${!i}" ; exit 1 ;;
    esac
    (( i++ ))
done 

build_exec=compiler
if [ -f $1 ]; then
    ./$build_exec $1

    if [ $? == 0 ]; then
        nasm tmp/out.asm -f elf64 -o tmp/out.o
        ld tmp/out.o -o tmp/out

        if ! (($compile_only)); then 
            tmp/out
            exit $?
        fi
    else
        echo "Compiler exited with error code $?"
    fi

else
    echo "$1: file not found"
fi