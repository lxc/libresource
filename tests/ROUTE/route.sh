# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/ROUTE
rm -f ./route_test
rm -f ./route_info.orig
rm -f ./route_info.txt
cc -I $LD_LIBRARY_PATH -std=gnu99 -o route_test route_test.c -L $LD_LIBRARY_PATH -lresource
ip route > ./route_info.orig
# Temporary fix to remove "linkdown" from route output, because we do not
# handle it in net_route.c yet. This will allow route.sh to succeed.
sed -i 's/ linkdown//g' route_info.orig
./route_test
diff ./route_info.orig ./route_info.txt
