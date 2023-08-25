# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/MISC
rm -f tests
rm -f ./vm_info.orig
cat /proc/vmstat > ./vm_info.orig
cc -I $LD_LIBRARY_PATH -std=gnu99 -o tests tests.c -L $LD_LIBRARY_PATH -lresource
./tests
