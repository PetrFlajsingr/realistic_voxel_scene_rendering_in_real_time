#!/bin/bash

output_dir='./cmake-build-debug/shaders/'
shader_dir='./shaders/'
ext='.spv'
file_names=`ls $shader_dir*`
for path in $file_names
do
   filename="$(basename $path)"
   out_path=$output_dir$filename$ext
   glslc $path -o $out_path
done


