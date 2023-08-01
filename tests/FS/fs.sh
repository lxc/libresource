# Enable #define TESTING and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/FS
cc -I $LD_LIBRARY_PATH -std=gnu99 -o fs_test fs_test.c -L $LD_LIBRARY_PATH -lresource
./fs_test
