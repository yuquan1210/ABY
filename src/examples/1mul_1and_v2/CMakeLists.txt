
add_executable(1mul_1and_v2 common_main.cpp common/1mul_1and_v2.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(1mul_1and_v2 ABY::aby ENCRYPTO_utils::encrypto_utils)
