#!/bin/bash

test_number=$1

../cmake-build-debug/ARQUIVOS < "in/$test_number.in" > tmp.txt
diff tmp.txt "out/$test_number.out"
rm tmp.txt
