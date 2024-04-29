
add_executable(1and_b2y_1mul common_main.cpp common/1and_b2y_1mul.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(1and_b2y_1mul ABY::aby ENCRYPTO_utils::encrypto_utils)
