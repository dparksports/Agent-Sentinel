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

# Executable Target
add_executable(agentic main.cpp)

# Link Libraries
target_link_libraries(agentic PRIVATE
    llama
    ggml
    ggml-base
    ggml-cpu
    ggml-cuda
)

# Increase stack size to 256MB to handle MSVC std::regex stack usage (Maximum attempt)
set_target_properties(agentic PROPERTIES LINK_FLAGS "/STACK:268435456")

# Post-build copy of DLLs
add_custom_command(TARGET agentic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/llama.dll"
    $<TARGET_FILE_DIR:agentic>
)
add_custom_command(TARGET agentic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml.dll"
    $<TARGET_FILE_DIR:agentic>
)
add_custom_command(TARGET agentic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml-base.dll"
    $<TARGET_FILE_DIR:agentic>
)
add_custom_command(TARGET agentic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml-cpu.dll"
    $<TARGET_FILE_DIR:agentic>
)
add_custom_command(TARGET agentic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LLAMA_BIN_DIR}/ggml-cuda.dll"
    $<TARGET_FILE_DIR:agentic>
)
