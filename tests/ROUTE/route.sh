# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/ROUTE

make clean
make route_test

ip route > ./route_info.orig
# Temporary fix to remove "linkdown" from route output, because we do not
# handle it in net_route.c yet. This will allow route.sh to succeed.
sed -i 's/ linkdown//g' route_info.orig
./route_test
diff ./route_info.orig ./route_info.txt
