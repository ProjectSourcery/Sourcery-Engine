if not exist build mkdir build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DDOUBLE_PRECISION=YES %*
echo Open build\SourceryEngine.sln to build the project.