#!/bin/bash

cur_dir=$(pwd)
src_dir=$cur_dir/../src
test_script=./make_test.sh
build_flags="env=test"

build=1

while getopts ":mxd" option; do
    case $option in
      m)
        test_script=./mem_test.sh;;
      x)
        build=0;;
      d)
        build_flags="env=debug";;
      /?)
        ;;
    esac
done

if [ $build == 1 ]; then
  cd "$src_dir" || exit
  make clean all $build_flags
  cd "$cur_dir" || exit
fi


for i in {1..16}
do
  echo "Testing $i"
  $test_script "$i"
  ec=$?
  if [ $ec != 0 ]; then
    echo "Failed test $i"
    exit $ec
  fi
done

if [ $build == 1 ]; then
  cd "$src_dir" || exit
  make clean
  cd "$cur_dir" || exit
fi
