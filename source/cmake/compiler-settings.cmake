# compiler-settings.cmake
# Shared compiler and language standard settings for all targets.
# Include this from each CMakeLists.txt that defines a target.

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include current dir by default
set(CMAKE_INCLUDE_CURRENT_DIR ON)
