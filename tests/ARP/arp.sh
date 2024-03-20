# Enable -DTESTING in Makefile and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/ARP

make clean
make arp_test

arp -n -a > arp_info.orig
#remove all "? " from above file
sed -i 's/? //g' arp_info.orig 
./arp_test
diff ./arp_info.orig ./arp_info.txt
