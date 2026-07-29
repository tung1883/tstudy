#!/usr/bin/env bash
# Builds every benchmark in this directory with full optimizations,
# matching the talk's own methodology (release build, default O3, no
# exotic flags). Run from anywhere; paths are relative to this script.
set -e
cd "$(dirname "$0")"

CXX=${CXX:-g++}
FLAGS="-O3 -std=c++17 -march=native"

for src in *.cpp; do
    name="${src%.cpp}"
    extra_libs=""
    if [[ "$src" == *false_sharing* ]]; then
        extra_libs="-lpthread"
    fi
    echo "Building $src -> $name.exe"
    "$CXX" $FLAGS "$src" -o "$name.exe" $extra_libs
done

echo "Done. Run any ./NN_name.exe to see its results."
