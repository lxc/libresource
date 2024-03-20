#!/bin/sh

# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/VM

make clean
make vm_test

cat /proc/vmstat > ./vm_info.orig
./vm_test
diff ./vm_info.orig ./vm_info.txt
