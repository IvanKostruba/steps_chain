add_executable(runExample example.cpp)
target_link_libraries(runExample PRIVATE steps_chain)
install(TARGETS runExample DESTINATION bin)