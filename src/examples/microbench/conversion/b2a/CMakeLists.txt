
add_executable(b2a b2a_test.cpp common/b2a.cpp ../../common/cost_calculator.cpp)
target_link_libraries(b2a ABY::aby ENCRYPTO_utils::encrypto_utils)
