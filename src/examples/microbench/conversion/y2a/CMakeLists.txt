
add_executable(y2a y2a_test.cpp common/y2a.cpp ../../common/cost_calculator.cpp)
target_link_libraries(y2a ABY::aby ENCRYPTO_utils::encrypto_utils)
