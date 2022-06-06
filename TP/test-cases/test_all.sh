#!/bin/bash

for i in {1..16}
do
  echo "Testing $i"
  ./make_test.sh $i
  if [ $? != 0 ]; then
    echo "Failed test $i"
    exit 1
  fi
done