FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(cuPoly REQUIRED)
FIND_PACKAGE(spogckks REQUIRED)

INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

#
# TEST
#
ENABLE_TESTING()

ADD_EXECUTABLE(
    spogckks_test
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
)

SET_PROPERTY(TARGET spogckks_test PROPERTY CUDA_STANDARD 11)

TARGET_LINK_LIBRARIES(
    spogckks_test
    ${GTEST_BOTH_LIBRARIES}
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# ADD_TEST(SPOGTests spog_test)
# GTEST_ADD_TESTS(
# 	TARGET cupoly_test)