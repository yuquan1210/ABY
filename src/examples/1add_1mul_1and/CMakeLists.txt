
add_executable(1add_1mul_1and common_main.cpp common/1add_1mul_1and.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(1add_1mul_1and ABY::aby ENCRYPTO_utils::encrypto_utils)
