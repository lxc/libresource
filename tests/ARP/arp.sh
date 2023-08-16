# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/ARP
rm -f ./arp_test
cc -I $LD_LIBRARY_PATH -std=gnu99 -o arp_test arp_test.c -L $LD_LIBRARY_PATH -lresource
rm -f ./arp_info.orig
rm -f ./arp_info.txt
arp -n -a > arp_info.orig
#remove all "? " from above file
sed -i 's/? //g' arp_info.orig 
./arp_test
diff ./arp_info.orig ./arp_info.txt
