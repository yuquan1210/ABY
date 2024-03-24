
add_executable(shl-clear op.cpp ../common/common_main.cpp ../common/common.cpp ../common/cost_estimator.cpp)
target_link_libraries(shl-clear ABY::aby ENCRYPTO_utils::encrypto_utils)
