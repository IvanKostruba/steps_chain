include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(runExample example.cpp)

install(TARGETS runExample DESTINATION bin)