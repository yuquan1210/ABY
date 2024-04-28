
add_executable(1mul_1and_v1 common_main.cpp common/1mul_1and_v1.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(1mul_1and_v1 ABY::aby ENCRYPTO_utils::encrypto_utils)
