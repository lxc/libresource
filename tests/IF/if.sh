# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/IF
rm -f ./if_test
rm -f ./if_info.txt
cc -I $LD_LIBRARY_PATH -std=gnu99 -o if_test if_test.c -L $LD_LIBRARY_PATH -lresource
./if_test
