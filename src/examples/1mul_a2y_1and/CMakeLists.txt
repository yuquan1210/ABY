
add_executable(1mul_a2y_1and common_main.cpp common/1mul_a2y_1and.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(1mul_a2y_1and ABY::aby ENCRYPTO_utils::encrypto_utils)
