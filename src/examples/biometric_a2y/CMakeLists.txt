
add_executable(biometric_a2y biometric_a2y.cpp common/biometric_a2y.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(biometric_a2y ABY::aby ENCRYPTO_utils::encrypto_utils)
