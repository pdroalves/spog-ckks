PROJECT (SPOGCKKS LANGUAGES CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.11.0)

SET(LIB_NAME "SPOGCKKS")
SET(PROJECT_VERSION_MAJOR "0")
SET(PROJECT_VERSION_MINOR "1")
SET(PROJECT_VERSION_PATCH "5")
SET(PROJECT_VERSION_TWEAK "6")
SET(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
SET(VERSION ${PROJECT_VERSION})

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
endif()

message("\n-- Configuring ${PROJECT_NAME} ${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})...\n")

SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CUDA_NVCC_FLAGS_DEBUG "-G")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

# configure a header file to pass some of the CMake SETtings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/SPOGCKKSConfig.h.in"
  "${PROJECT_SOURCE_DIR}/include/SPOGCKKS/SPOGCKKSConfig.h"
  )

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_STANDARD 11)
SET(GCC_CUDA_VERSION "gcc" CACHE STRING "GCC compiler to be used with cuda")
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

SET(CUDA_SEPARABLE_COMPILATION ON)

ENABLE_LANGUAGE(CUDA)
FIND_PACKAGE(CUDA REQUIRED)
FIND_PACKAGE(NTL 11.0 REQUIRED)
FIND_PACKAGE(GMP 6.0)
FIND_PACKAGE(cuPoly 0.2.3 REQUIRED)
FIND_PACKAGE(OpenMP)
FIND_PACKAGE(cxxopts REQUIRED)
FIND_PACKAGE(Rapidjson REQUIRED)
FIND_PACKAGE(Doxygen)

LINK_DIRECTORIES(/usr/local/cuda/lib64)

if (OPENMP_FOUND)
    SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
LIST(APPEND CUDA_NVCC_FLAGS -Wno-deprecated-gpu-targets)
LIST(APPEND CUDA_NVCC_FLAGS --default-stream per-thread)
LIST(APPEND CUDA_NVCC_FLAGS --relocatable-device-code=true)
LIST(APPEND CUDA_NVCC_FLAGS --use_fast_math)

ENABLE_TESTING()
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#
# Lib
#

SET ( 
    libspogheaders 
    include/SPOGCKKS/ckks.h
    include/SPOGCKKS/ckkscontext.h
    include/SPOGCKKS/SPOGCKKSConfig.h
    include/SPOGCKKS/arithmetic/ciphertext.h
    include/SPOGCKKS/tool/encoder.h
    include/SPOGCKKS/tool/version.h
)

INCLUDE_DIRECTORIES(
    include
    ${CUDA_INCLUDE_DIRS}
    ${CUDA_CURAND_INCLUDE_DIRS}
    ${CUPOLY_INCLUDE_DIR}
)

SET ( 
    libsrc 
    src/ckks.cpp
    src/keys.cpp
    src/ckkscontext.cpp
    src/arithmetic/ciphertext.cpp
    src/tool/encoder.cu
    src/tool/version.cpp
)

CUDA_ADD_LIBRARY(spogckks STATIC ${libsrc})
SET_TARGET_PROPERTIES(spogckks PROPERTIES CUDA_ARCHITECTURES native)

TARGET_LINK_LIBRARIES(
    spogckks
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    -lfftw3
    -lm
    -lssl
    -lcrypto
    ${CUDA_curand_LIBRARY}
)

#
# Demos
#
ADD_SUBDIRECTORY(demos)

#
# TEST
#
ENABLE_TESTING()
ADD_SUBDIRECTORY(tests)

#
# Documentation
#
# INCLUDE(cmake/doxygen.cmake)

#
# Install
# 

INSTALL(TARGETS spogckks
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

INSTALL(FILES include/SPOGCKKS/tool/version.h include/SPOGCKKS/tool/encoder.h
    DESTINATION include/SPOGCKKS/tool
    )

INSTALL(FILES include/SPOGCKKS/ckks.h include/SPOGCKKS/ckkscontext.h include/SPOGCKKS/keys.h include/SPOGCKKS/SPOGCKKSConfig.h
    DESTINATION include/SPOGCKKS
    )

INSTALL(FILES include/SPOGCKKS/arithmetic/ciphertext.h
    DESTINATION include/SPOGCKKS/arithmetic)
