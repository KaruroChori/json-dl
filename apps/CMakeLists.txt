file(GLOB DIRS */CMakeLists.txt)

foreach(T ${DIRS})
	get_filename_component(TS ${T} DIRECTORY)
	add_subdirectory(${TS})
endforeach(T)
