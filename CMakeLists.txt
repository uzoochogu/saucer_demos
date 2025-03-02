cmake_minimum_required(VERSION 3.28)
project(saucer_demos LANGUAGES CXX VERSION 1.0)

# --------------------------------------------------------------------------------------------------------
# Build Configuration 
# --------------------------------------------------------------------------------------------------------
set(WASM_BUILD_DIR "${CMAKE_BINARY_DIR}/wasm-builds")
set(EMBED_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/generated")

include_directories(
    ${EMBED_OUTPUT_DIR}
    ${CMAKE_SOURCE_DIR}
)

# --------------------------------------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    saucer
    GIT_REPOSITORY "https://github.com/saucer/saucer"
    GIT_TAG v4.2.0
)
FetchContent_MakeAvailable(saucer)

# --------------------------------------------------------------------------------------------------------
# Build Functions
# --------------------------------------------------------------------------------------------------------

# Compiles C++ source to WebAssembly using emscripten
function(add_wasm_target TARGET_NAME SOURCE_FILE OUTPUT_DIR)
    set(OUTPUT_PATH "${WASM_BUILD_DIR}/${OUTPUT_DIR}/src/app.js")
    string(REPLACE " " ";" EMCC_FLAGS_LIST "${ARGN}")
    
    add_custom_command(
        OUTPUT ${OUTPUT_PATH}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${WASM_BUILD_DIR}/${OUTPUT_DIR}/src"
        # Copy index.html and all static files to the build directory
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
            "${CMAKE_SOURCE_DIR}/${OUTPUT_DIR}/"
            "${WASM_BUILD_DIR}/${OUTPUT_DIR}/"
        COMMAND emcc ${SOURCE_FILE} -o ${OUTPUT_PATH} ${EMCC_FLAGS_LIST}
        DEPENDS 
            ${SOURCE_FILE}
            "${CMAKE_SOURCE_DIR}/${OUTPUT_DIR}/"
        COMMENT "Building WebAssembly target ${TARGET_NAME}"
        VERBATIM
        USES_TERMINAL
    )
    
    add_custom_target(${TARGET_NAME}_wasm DEPENDS ${OUTPUT_PATH})
endfunction()

# Embeds WebAssembly resources using saucer-cli
function(add_saucer_embed TARGET_NAME WASM_DIR OUTPUT_DIR)
    set(EMBED_PATH "${EMBED_OUTPUT_DIR}/${OUTPUT_DIR}")
    
    add_custom_command(
        OUTPUT "${EMBED_PATH}/all.hpp"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${EMBED_PATH}"
        COMMAND saucer embed "${WASM_BUILD_DIR}/${WASM_DIR}/" "${EMBED_PATH}"
        DEPENDS ${TARGET_NAME}_wasm
        COMMENT "Embedding resources for ${TARGET_NAME}"
        VERBATIM
        USES_TERMINAL
    )
    
    add_custom_target(${TARGET_NAME}_embed DEPENDS "${EMBED_PATH}/all.hpp")
endfunction()

# --------------------------------------------------------------------------------------------------------
# WebAssembly Targets
# --------------------------------------------------------------------------------------------------------

# Basic WASM example
add_wasm_target(wasm 
    "${CMAKE_SOURCE_DIR}/emscripten-src/wasm_app.cpp" 
    "wasm-content"
    "-s WASM=1 -s EXPORTED_FUNCTIONS=['_runWasmFunction'] -s MODULARIZE=1 -s EXPORT_NAME=createModule -lembind -s USE_WEBGPU=1"
)

# WebGPU example with JS bindings
add_wasm_target(wasm_webgpu_js 
    "${CMAKE_SOURCE_DIR}/emscripten-src/wasm_webgpu_app_js.cpp" 
    "wasm-webgpu-js-content"
    "-s WASM=1 -s USE_WEBGPU=1 -s NO_EXIT_RUNTIME=1 -s JSPI=1 -s EXPORTED_RUNTIME_METHODS=['UTF8ToString'] -O3 --bind"
)

# WebGPU example with C++ API
add_wasm_target(wasm_webgpu 
    "${CMAKE_SOURCE_DIR}/emscripten-src/wasm_webgpu_app.cpp" 
    "wasm-webgpu-content"
    "-s WASM=1 -s USE_WEBGPU=1 -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS=['_runWasmFunction'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -O3 --bind"
)

# --------------------------------------------------------------------------------------------------------
# Resource Embedding
# --------------------------------------------------------------------------------------------------------
add_saucer_embed(wasm "wasm-content" "wasm")
add_saucer_embed(wasm_webgpu_js "wasm-webgpu-js-content" "wasm-webgpu-js")
add_saucer_embed(wasm_webgpu "wasm-webgpu-content" "wasm-webgpu")

# --------------------------------------------------------------------------------------------------------
# Native Executables
# --------------------------------------------------------------------------------------------------------
add_executable(wasm_app saucer-src/wasm.cpp)
add_executable(wasm_webgpu_js_app saucer-src/wasm_webgpu_js.cpp)
add_executable(wasm_webgpu_app saucer-src/wasm_webgpu.cpp)

foreach(TARGET wasm_app wasm_webgpu_js_app wasm_webgpu_app)
    target_compile_features(${TARGET} PRIVATE cxx_std_23)
    set_target_properties(${TARGET} PROPERTIES 
        CXX_STANDARD 23 
        CXX_EXTENSIONS OFF 
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(${TARGET} PRIVATE saucer::saucer)
endforeach()

# Link dependencies
add_dependencies(wasm_app wasm_embed)
add_dependencies(wasm_webgpu_js_app wasm_webgpu_js_embed)
add_dependencies(wasm_webgpu_app wasm_webgpu_embed)
