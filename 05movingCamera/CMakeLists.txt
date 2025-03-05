cmake_minimum_required(VERSION 3.10)

# Set the project name
project(TinyRenderer)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)

# Gather all the source files in the current directory
file(GLOB SOURCES "*.cpp")

# Create the executable from the source files
add_executable(main ${SOURCES})

# Link the math library (equivalent to -lm in the Makefile)
target_link_libraries(main m)

# Optionally, you can specify additional compile flags if needed
target_compile_options(main PRIVATE -Wall)
