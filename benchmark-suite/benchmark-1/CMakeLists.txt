set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(benchmark-1 main.cpp)
target_link_libraries(benchmark-1 ${LIBS} ${LOC_LIBS})
