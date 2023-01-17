# Sourcery Engine

The first ever engine that **will** be based upon the [LuaU language](https://luau-lang.org/)

## TODO

- [x] Textures
- [ ] ECS (Entity Component System)
- [x] Organize the project
- [ ] Implement LuaU into the engine
- [ ] Optimalizations
- [ ] Multisampling
- [ ] Imgui?

## Building

### Unix Build Instructions

- Install the dependencies: cmake, glm, vulkan and glfw

- For example
  ```
    sudo apt install vulkan-tools
    sudo apt install libvulkan-dev
    sudo apt install vulkan-validationlayers-dev spirv-tools
    sudo apt install libglfw3-dev
    sudo apt install libglm-dev
    sudo apt install cmake
  ```
- To Build
  ```
   cd LittleVulkanEngine
   ./unixBuild.sh
  ```

### MacOS Build Intructions

#### Install Dependencies

- [Download and install MacOS Vulkan sdk](https://vulkan.lunarg.com/)
- [Download and install Homebrew](https://brew.sh/)

- Then in a terminal window

  ```
    brew install cmake
    brew install glfw
    brew install glm
  ```

- To Build
  ```
   cd Sourcery-Engine
   ./unixBuild.sh
  ```

### Windows Build Instructions

- [Download and install Windows Vulkan sdk](https://vulkan.lunarg.com/)
- [Download and install Windows cmake x64 installer](https://cmake.org/download/)
  - Make sure to select "Add cmake to the system Path for all users"
- [Download GLFW](https://www.glfw.org/download.html) (64-bit precompiled binary)
- [Download GLM](https://github.com/g-truc/glm/releases)
- Download and open the project and rename ".env.example.win32.cmake" to ".env.cmake"
- Update the filepath variables in .env.cmake to your installation locations

#### Building for Visual Studio 2019

- In windows powershell

```
 cd Sourcery-Engine
 ./buildWin32VS2022.bat
```

- If cmake finished successfully, it will create a SourceryEngine.sln file in the build directory that can be opened with visual studio. In visual studio right click the Shaders project -> Build, to build the shaders. Right click the SourceryEngine project -> Set as Startup Project. Change from debug to release, and then build and start without debugging.

#### Building for minGW

- [Download and install MinGW-w64](https://www.mingw-w64.org/downloads/), you probably want MingW-W64-builds/
- Make sure MinGW has been added to your Path
- Also set MINGW_PATH variable in the project's .env.cmake
- In windows powershell

```
 cd Sourcery-Engine
 ./buildWin32MinGW.bat
```

- This will build the project to build/SourceryEngine.exe, double click in file explorer to open and run
