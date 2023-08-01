#!/bin/sh

# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/VM
rm vm_test
cc -I $LD_LIBRARY_PATH -std=gnu99 -o vm_test vm_test.c -L $LD_LIBRARY_PATH -lresource
rm ./vm_info.orig
rm ./vm_info.txt
cat /proc/vmstat > ./vm_info.orig
./vm_test
diff ./vm_info.orig ./vm_info.txt
