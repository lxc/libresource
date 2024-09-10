# Enable #define TESTING and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/FS

make clean
make fs_test

./fs_test
