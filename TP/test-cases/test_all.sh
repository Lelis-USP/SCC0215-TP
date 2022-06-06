#!/bin/bash

cur_dir=$(pwd)
src_dir=$cur_dir/../src

cd $src_dir
make clean all

cd $cur_dir
for i in {1..16}
do
  echo "Testing $i"
  ./make_test.sh $i
  if [ $? != 0 ]; then
    echo "Failed test $i"
    exit 1
  fi
done

cd $src_dir
make clean
