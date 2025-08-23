cmake_minimum_required(VERSION 3.20)
project(DotEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Find SDL3 packages
find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE SOURCES
  "DotEngine/*.cpp"
  "DotEngine/*.c"
)

file(GLOB_RECURSE HEADERS
  "DotEngine/*.hpp"
  "DotEngine/*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
  DotEngine
  ${GLM_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} 
  SDL3
  SDL3_ttf
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
  target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(${PROJECT_NAME} PRIVATE -O3)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")
endif()

# Enable more compiler warnings
target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall -Wextra -Wpedantic -march=native
)
