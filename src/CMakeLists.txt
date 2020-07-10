set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES ./*.cpp)
add_library(local_lib ${SOURCES})
target_link_libraries(local_lib ${LIBS} ${LOC_LIBS})

set(LOC_LIBS ${LOC_LIBS} "local_lib" CACHE INTERNAL "LOC_LIBS")
