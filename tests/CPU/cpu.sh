# Enable #define TESTING and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH
cd tests/CPU
rm -f cpu_test
cc -I $LD_LIBRARY_PATH -std=gnu99 -o cpu_test cpu_test.c -L $LD_LIBRARY_PATH -lresource
rm -f ./cpu_info.orig
rm -f ./cpu_info.txt
cat /proc/cpuinfo > ./cpu_info.orig
./cpu_test
diff ./cpu_info.orig ./cpu_info.txt
