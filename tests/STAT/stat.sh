export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/STAT

make clean
make stat_test

cat /proc/stat > stat_info.orig
./stat_test
diff stat_info.orig stat_info.txt

