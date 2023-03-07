set(GLFW_PATH X:/dev/Libraries/glfw-3.3.4.bin.WIN64)
set(GLM_PATH X:/dev/Libraries/glm)
set(VULKAN_SDK_PATH  X:/VulkanSDK/1.2.182.0)

# # Uncomment this if using buildWin32MinGW.bat and not VisualStudio20XX
# set(CMAKE_SYSTEM_NAME Windows)
# set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
# set(MINGW_PATH "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64") # change path here if not using MSYS2

# # cross compilers to use for C, C++ and Fortran
# set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
# set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
# set(CMAKE_Fortran_COMPILER ${TOOLCHAIN_PREFIX}-gfortran)
# set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# # target environment on the build host system
# set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# # modify default behavior of FIND_XXX() commands
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

## EXTERNAL LIBS ##

# Optional set TINYOBJ_PATH to target specific version, otherwise defaults to external/tinyobjloader
# set(TINYOBJ_PATH X:/dev/Libraries/tinyobjloader)

# Optional set STB_PATH to target specific version, otherwise defaults to external/stb
# set(STB_PATH X:/dev/Libraries/stb)

# Optional set IMGUI_PATH to target specific version, otherwise defaults to external/imgui
# set(IMGUI_PATH X:/dev/Libraries/imgui)