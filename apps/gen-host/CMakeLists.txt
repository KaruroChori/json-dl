set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gen-host main.cpp)
target_link_libraries(gen-host ${LIBS} ${LOC_LIBS})
