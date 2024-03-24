
add_executable(1add_1mul 1add_1mul_test.cpp common/1add_1mul.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(1add_1mul ABY::aby ENCRYPTO_utils::encrypto_utils)
