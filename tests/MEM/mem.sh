#!/bin/sh

# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/MEM
rm -f mem_test
rm -f mem_info.orig
rm -f mem_info.txt
cc -I $LD_LIBRARY_PATH -std=gnu99 -o mem_test mem_test.c -L $LD_LIBRARY_PATH -lresource
cat /proc/meminfo > mem_info.orig
./mem_test
sed -i 's/[ ]\+/ /g' mem_info.orig
diff mem_info.orig mem_info.txt
