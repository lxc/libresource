# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/MISC

make clean
make tests

cat /proc/vmstat > ./vm_info.orig
./tests
