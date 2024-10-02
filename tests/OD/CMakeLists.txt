#target_compile_options(test2 PRIVATE -fsanitize=address,undefined)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

file(GLOB inputs "inputs*.cpp")
foreach(file ${inputs})
	string(REGEX MATCH "inputs-((.*)[0-9]+)\.cpp" _ ${file})
	message(STATUS "Generating: ${file} ${CMAKE_MATCH_1} ${CMAKE_MATCH_2}")

	add_custom_target(test-${CMAKE_MATCH_1}
		COMMAND python  ../../mptc --reduction reflexivity --reduction symmetry --debug --exit-on-error
                        --overwrite-file main.cpp --out-dir "/tmp/${CMAKE_MATCH_1}" od.mpt ${file} main.cpp
		COMMAND cd "/tmp/${CMAKE_MATCH_1}/" && cmake .
		COMMAND make -C "/tmp/${CMAKE_MATCH_1}/"
		COMMAND cp "/tmp/${CMAKE_MATCH_1}/monitor" test-${CMAKE_MATCH_1})
	add_dependencies(check test-${CMAKE_MATCH_1})

	add_test(NAME test-${CMAKE_MATCH_1}
		 COMMAND test-${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
endforeach()

