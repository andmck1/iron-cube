#! /usr/bin/zsh
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cmake --build build --target ironcube_docs
