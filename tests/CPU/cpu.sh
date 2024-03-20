# Enable #define TESTING and the recompile library - make
export LD_LIBRARY_PATH=`git rev-parse --show-toplevel`
cd $LD_LIBRARY_PATH/tests/CPU

make clean
make cpu_test

cat /proc/cpuinfo > ./cpu_info.orig
./cpu_test
diff ./cpu_info.orig ./cpu_info.txt
