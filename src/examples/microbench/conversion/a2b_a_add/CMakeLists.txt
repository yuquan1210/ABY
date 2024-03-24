
add_executable(a2b_a_add a2b_a_add_test.cpp common/a2b_a_add.cpp)
target_link_libraries(a2b_a_add ABY::aby ENCRYPTO_utils::encrypto_utils)
