
add_executable(sub op.cpp ../common/common_main.cpp ../common/common.cpp ../common/cost_calculator.cpp)
target_link_libraries(sub ABY::aby ENCRYPTO_utils::encrypto_utils)
