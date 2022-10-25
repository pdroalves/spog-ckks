FIND_PACKAGE(cuPoly REQUIRED)
FIND_PACKAGE(spogckks REQUIRED)


# demo: simple_mul
CUDA_ADD_EXECUTABLE(
    simple_mul
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_mul.cpp
)

TARGET_LINK_LIBRARIES(
    simple_mul
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)


# demo: simple_mul_const
CUDA_ADD_EXECUTABLE(
    simple_mul_const
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_mul_const.cpp
)

TARGET_LINK_LIBRARIES(
    simple_mul_const
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)
# demo: add_mul
CUDA_ADD_EXECUTABLE(
    add_mul
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/add_mul.cpp
)

TARGET_LINK_LIBRARIES(
    add_mul
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)
# demo: merge
CUDA_ADD_EXECUTABLE(
    merge
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/merge.cpp
)

TARGET_LINK_LIBRARIES(
    merge
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: rotate
CUDA_ADD_EXECUTABLE(
    rotate
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/rotate.cpp
)

TARGET_LINK_LIBRARIES(
    rotate
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: conjugate
CUDA_ADD_EXECUTABLE(
    conjugate
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/conjugate.cpp
)

TARGET_LINK_LIBRARIES(
    conjugate
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: sequencial_mul
CUDA_ADD_EXECUTABLE(
    sequential_mul
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/sequential_mul.cpp
)

TARGET_LINK_LIBRARIES(
    sequential_mul
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: toy_innerprod
CUDA_ADD_EXECUTABLE(
    toy_innerprod
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/toy_innerprod.cpp
)

TARGET_LINK_LIBRARIES(
    toy_innerprod
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: sequencial_add
CUDA_ADD_EXECUTABLE(
    sequential_add
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/sequential_add.cpp
)

TARGET_LINK_LIBRARIES(
    sequential_add
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: encrypt_decrypt
CUDA_ADD_EXECUTABLE(
    encrypt_decrypt
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/encrypt_decrypt.cpp
)

TARGET_LINK_LIBRARIES(
    encrypt_decrypt
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: importexport
CUDA_ADD_EXECUTABLE(
    importexport
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/importexport.cpp
)

TARGET_LINK_LIBRARIES(
    importexport
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: debug
# CUDA_ADD_EXECUTABLE(
#     debug
#     EXCLUDE_FROM_ALL
#     ${CMAKE_CURRENT_SOURCE_DIR}/debug.cpp
# )

# TARGET_LINK_LIBRARIES(
#     debug
#     -lcupolyckks
#     -lntl 
#     -lgmp 
#     -fopenmp
#     ${CUDA_curand_LIBRARY}
#     spogckks
# )

# demo: encode_decode
CUDA_ADD_EXECUTABLE(
    encode_decode
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/encode_decode.cpp
)

TARGET_LINK_LIBRARIES(
    encode_decode
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: range
CUDA_ADD_EXECUTABLE(
    range
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/range.cpp
)

TARGET_LINK_LIBRARIES(
    range
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: simple_add
CUDA_ADD_EXECUTABLE(
    simple_add
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_add.cpp
)

TARGET_LINK_LIBRARIES(
    simple_add
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# demo: simple_add
CUDA_ADD_EXECUTABLE(
    depth
    EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/depth.cpp
)

TARGET_LINK_LIBRARIES(
    depth
    -lcupolyckks
    -lntl 
    -lgmp 
    -fopenmp
    ${CUDA_curand_LIBRARY}
    spogckks
)

# custom target
ADD_CUSTOM_COMMAND(
    OUTPUT demos.out
    DEPENDS encrypt_decrypt depth conjugate range merge rotate encode_decode simple_add toy_innerprod sequential_mul sequential_add simple_mul add_mul simple_mul_const
    COMMAND ${CMAKE_COMMAND} -E touch demos.out
    )
ADD_CUSTOM_TARGET(
    demos
    DEPENDS demos.out
    )