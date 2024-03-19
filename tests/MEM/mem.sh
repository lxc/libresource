#!/bin/sh

# Enable -DTESTING in Makefile and the recompile library - make

export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/MEM

make clean
make mem_test

cat /proc/meminfo > mem_info.orig
sed -i 's/[ ]\+/ /g' mem_info.orig
./mem_test
diff mem_info.orig mem_info.txt

#make mem_test_cg
#./mem_test_cg
