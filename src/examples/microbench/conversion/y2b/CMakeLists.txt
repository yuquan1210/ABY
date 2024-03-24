
add_executable(y2b y2b_test.cpp common/y2b.cpp ../../common/cost_calculator.cpp)
target_link_libraries(y2b ABY::aby ENCRYPTO_utils::encrypto_utils)
