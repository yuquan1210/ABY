
add_executable(xor op.cpp ../common/common_main.cpp ../common/common.cpp ../common/cost_calculator.cpp)
target_link_libraries(xor ABY::aby ENCRYPTO_utils::encrypto_utils)
