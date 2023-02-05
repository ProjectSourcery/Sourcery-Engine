if not exist build mkdir build
cd build
cmake -S ../ -B . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
make -j 8 && make Shaders
cd ..