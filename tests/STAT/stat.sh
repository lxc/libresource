export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/STAT
rm -f stat_info.orig
rm -f stat_info.txt
cc -I $LD_LIBRARY_PATH -std=gnu99 -o stat_test stat_test.c -L $LD_LIBRARY_PATH -lresource
cat /proc/stat > stat_info.orig
./stat_test
#diff stat_info.orig stat_info.txt

