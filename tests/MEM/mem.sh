#!/bin/sh

# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=../..
cd $LD_LIBRARY_PATH
cd tests/MEM
rm mem_test
cc -I $LD_LIBRARY_PATH -std=gnu99 -o mem_test mem_test.c -L $LD_LIBRARY_PATH -lresource
cat /proc/meminfo > mem_info1.orig
./mem_test
cat ./mem_info1.orig | sed 's/[ ]\+/ /g' > mem_info.orig
diff mem_info.orig mem_info.txt
