find_program (BASH_PROGRAM bash)
if (BASH_PROGRAM)
  add_test (test-1 ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/make.sh)
endif (BASH_PROGRAM)
