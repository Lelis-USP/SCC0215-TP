#!/bin/bash

test_number=$1

valgrind_args="--leak-check=full --show-leak-kinds=all --error-exitcode=1 --exit-on-first-error=no -q"
# shellcheck disable=SC2086
valgrind $valgrind_args ../src/main < "in/$test_number.in" > tmp.txt
ec=$?
if [ $ec != 0 ]; then
  exit $ec
fi

diff tmp.txt "out/$test_number.out"
ec=$?
if [ $ec == 0 ]; then
  rm tmp.txt
fi
exit $ec
