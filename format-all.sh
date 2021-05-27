#!/bin/sh

for folder in src
do
  find ${folder} -regex '.*\.\(c\|cpp\|h\)' -exec sed -i "s/#pragma omp/\\/\\/#pragma omp/g" {} \;
  find ${folder} -regex '.*\.\(c\|cpp\|h\)' -exec clang-format -style=file -i {} \;
  find ${folder} -regex '.*\.\(c\|cpp\|h\)' -exec sed -i "s/\\/\\/ *#pragma omp/#pragma omp/g" {} \;
done

