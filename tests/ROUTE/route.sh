# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/ROUTE
rm -f ./route_test
cc -I $LD_LIBRARY_PATH -std=gnu99 -o route_test route_test.c -L $LD_LIBRARY_PATH -lresource
rm -f ./route_info.orig
rm -f ./route_info.txt
ip route > ./route_info.orig
./route_test
diff ./route_info.orig ./route_info.txt
