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
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/steps_chain>
	$<INSTALL_INTERFACE:include/steps_chain>
)
target_compile_features(steps_chain INTERFACE cxx_std_17)

include(CMakePackageConfigHelpers)
set(CONFIG_PACKAGE_INSTALL_DIR lib/cmake/steps_chain)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/steps_chain-config-version.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMinorVersion
)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/steps_chain.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/steps_chain-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/steps_chain DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS steps_chain
        EXPORT steps_chainTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT steps_chainTargets DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/steps_chain-config-version.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/steps_chain-config.cmake"
        DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR})

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(STEPS_CHAIN_BUILD_TESTS "Build all tests." OFF)
option(STEPS_CHAIN_BUILD_EXAMPLE "Build example." OFF)

if (STEPS_CHAIN_BUILD_EXAMPLE)
	add_subdirectory(example)
endif()
if (STEPS_CHAIN_BUILD_TESTS)
	add_subdirectory(test)
endif()