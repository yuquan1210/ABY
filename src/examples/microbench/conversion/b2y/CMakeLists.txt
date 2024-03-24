
add_executable(b2y b2y_test.cpp common/b2y.cpp ../../common/cost_calculator.cpp)
target_link_libraries(b2y ABY::aby ENCRYPTO_utils::encrypto_utils)
