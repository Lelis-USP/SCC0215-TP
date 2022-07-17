#!/bin/bash

test_number=$1

#../cmake-build-debug/ARQUIVOS < "in/$test_number.in" > tmp.txt
../src/main < "in/$test_number.in" > tmp.txt
diff tmp.txt "out/$test_number.out"
ec=$?
if [ $ec == 0 ]; then
  rm tmp.txt
fi
exit $ec
