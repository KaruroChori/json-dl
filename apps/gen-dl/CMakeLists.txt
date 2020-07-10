set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gen-dl main.cpp)
target_link_libraries(gen-dl ${LIBS} ${LOC_LIBS})
