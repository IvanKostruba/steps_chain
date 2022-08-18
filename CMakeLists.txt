cmake_minimum_required(VERSION 3.14)

project(STEPS_CHAIN
	VERSION 1.0.0
	DESCRIPTION "Header-only library providing resumable sequences of functions."
	HOMEPAGE_URL "https://github.com/IvanKostruba/steps_chain"
	LANGUAGES CXX)

include(GNUInstallDirs)

add_library(steps_chain INTERFACE)
target_include_directories(
	steps_chain
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(steps_chain INTERFACE cxx_std_17)

include(CMakePackageConfigHelpers)
set(CONFIG_PACKAGE_INSTALL_DIR lib/cmake/steps_chain)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/steps_chain-config-version.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMinorVersion
)

install(DIRECTORY include DESTINATION include)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(STEPS_CHAIN_BUILD_TESTS "Build all tests." OFF)
option(STEPS_CHAIN_BUILD_EXAMPLE "Build example." OFF)

if (STEPS_CHAIN_BUILD_EXAMPLE)
	add_subdirectory(example)
endif()
if (STEPS_CHAIN_BUILD_TESTS)
	add_subdirectory(test)
endif()