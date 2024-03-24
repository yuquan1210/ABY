
add_executable(1mul_1add 1mul_1add_test.cpp common/1mul_1add.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(1mul_1add ABY::aby ENCRYPTO_utils::encrypto_utils)
