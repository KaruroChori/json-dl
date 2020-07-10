cmake_minimum_required(VERSION 3.15)
project(framework)

set(LOC_LIBS "" CACHE INTERNAL "LOC_LIBS")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN YES)
set(DOXYGEN_EXCLUDE_PATTERNS */opt/*)

doxygen_add_docs(doxygen)

add_subdirectory(opt)

include_directories(headers)

add_subdirectory(src)
add_subdirectory(apps)

add_subdirectory(benchmark-suite)

enable_testing()
add_subdirectory(test-suite)
