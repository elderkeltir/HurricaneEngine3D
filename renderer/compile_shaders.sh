#!/bin/bash

rm -rf shaders/*.spv
for filename in shaders/*.glsl; do
    glslangValidator -V "${filename}" -o "${filename%.*}.spv"
done