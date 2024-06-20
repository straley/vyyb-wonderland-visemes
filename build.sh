#!/bin/bash

cd emsdk
source ./emsdk_env.sh
emcc ../src/viseme_detection.c -O3 -s EXPORTED_FUNCTIONS="['_malloc', '_free', '_detectVisemes', '_classifyPhoneme']" -o ../build/viseme_detection.js
cp ../build/viseme_detection.wasm ../site/
cp ../build/viseme_detection.js ../site/
