
add_executable(a2b a2b_test.cpp common/a2b.cpp ../../common/cost_calculator.cpp)
target_link_libraries(a2b ABY::aby ENCRYPTO_utils::encrypto_utils)
