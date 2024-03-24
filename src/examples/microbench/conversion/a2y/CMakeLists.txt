
add_executable(a2y a2y_test.cpp common/a2y.cpp ../../common/cost_calculator.cpp)
target_link_libraries(a2y ABY::aby ENCRYPTO_utils::encrypto_utils)
