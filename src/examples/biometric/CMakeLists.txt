
add_executable(biometric biometric.cpp common/biometric.cpp ../microbench/common/cost_calculator.cpp)
target_link_libraries(biometric ABY::aby ENCRYPTO_utils::encrypto_utils)
