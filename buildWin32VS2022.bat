if not exist build mkdir build
cd build
cmake -S . -B VS2022_CL -G "Visual Studio 17 2022" -A x64 %*
echo Open VS2022_CL\HelloWorld.sln to build the project.
cd ..