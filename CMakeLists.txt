cmake_minimum_required(VERSION 3.10)
project(AgentSentinel)

set(CMAKE_CXX_STANDARD 17)

# Define paths to llama.cpp build
set(LLAMA_CPP_ROOT "${CMAKE_SOURCE_DIR}/../llama.cpp")
set(LLAMA_INCLUDE_DIR "${LLAMA_CPP_ROOT}/include")
set(GGML_INCLUDE_DIR "${LLAMA_CPP_ROOT}/ggml/include")
set(LLAMA_LIB_DIR "${LLAMA_CPP_ROOT}/build/src")
set(GGML_LIB_DIR "${LLAMA_CPP_ROOT}/build/ggml/src")
set(GGML_CUDA_LIB_DIR "${LLAMA_CPP_ROOT}/build/ggml/src/ggml-cuda")
set(LLAMA_BIN_DIR "${LLAMA_CPP_ROOT}/build/bin") 

include_directories(${LLAMA_INCLUDE_DIR} ${GGML_INCLUDE_DIR})
link_directories(${LLAMA_LIB_DIR} ${GGML_LIB_DIR} ${GGML_CUDA_LIB_DIR})

add_executable(AgentSentinel main.cpp)

# Link against llama.lib and ggml.lib
target_link_libraries(AgentSentinel PRIVATE llama ggml ggml-base ggml-cpu ggml-cuda)

# Increase stack size to 256MB to handle MSVC std::regex stack usage (Maximum attempt)
set_target_properties(AgentSentinel PROPERTIES LINK_FLAGS "/STACK:268435456")

# Copy dlls to output
add_custom_command(TARGET AgentSentinel POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/llama.dll"
    "$<TARGET_FILE_DIR:AgentSentinel>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml.dll"
    "$<TARGET_FILE_DIR:AgentSentinel>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml-base.dll"
    "$<TARGET_FILE_DIR:AgentSentinel>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml-cpu.dll"
    "$<TARGET_FILE_DIR:AgentSentinel>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml-cuda.dll"
    "$<TARGET_FILE_DIR:AgentSentinel>"
)
