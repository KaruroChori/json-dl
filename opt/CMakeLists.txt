file(GLOB DIRS ./*/CMakeLists.txt)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(${BUILD_CPR_TESTS} OFF)

foreach(T ${DIRS})
	get_filename_component(TS ${T} DIRECTORY)
	add_subdirectory(${TS})
endforeach(T)

set(LOC_LIBS ${LOC_LIBS} "nlohmann_json::nlohmann_json" CACHE INTERNAL "LOC_LIBS")
set(LOC_LIBS ${LOC_LIBS} "cpr" CACHE INTERNAL "LOC_LIBS")
