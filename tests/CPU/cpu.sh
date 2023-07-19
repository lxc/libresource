# Enable #define TESTING and the recompile library - make
export LD_LIBRARY_PATH=../..
cd $LD_LIBRARY_PATH
cd tests/CPU
cc -I $LD_LIBRARY_PATH -std=gnu99 -o cpu_test cpu_test.c -L $LD_LIBRARY_PATH -lresource
rm ./cpu_info.orig
rm ./cpu_info.txt
cat /proc/cpuinfo > ./cpu_info.orig
./cpu_test
diff ./cpu_info.orig ./cpu_info.txt
