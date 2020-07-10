set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(impl main.cpp)
target_link_libraries(impl ${LIBS} ${LOC_LIBS})
