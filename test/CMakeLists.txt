if (test)

	include(FetchContent)
	FetchContent_Declare(
	  googletest
	  # Specify the commit you depend on and update it regularly.
	  URL https://github.com/google/googletest/archive/refs/heads/main.zip
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)

	enable_testing()

	include_directories(${PROJECT_SOURCE_DIR}/include)

	add_executable(runTests "raw_chain_tests.cpp" "raw_context_chain_tests.cpp")

	target_link_libraries(runTests PRIVATE gtest_main)

	add_test(NAME unitTests COMMAND runTests)

endif()