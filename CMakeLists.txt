cmake_minimum_required (VERSION 3.20)

project (RayFlow LANGUAGES CXX C)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

option(RAYFLOW_BUILD_GPU_RENDERER "Build GPU renderer" OFF)
option(RAYFLOW_USE_FLOAT_AS_DOUBLE "Use 64-bits floats" OFF)

if (RAYFLOW_USE_FLOAT_AS_DOUBLE)
    list(APPEND RAYFLOW_MACRO_DEFINITIONS "RAYFLOW_USE_FLOAT_AS_DOUBLE")
endif()

if (MSVC)
    list(APPEND RAYFLOW_MACRO_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")
endif()

add_library(rayflow_warnings INTERFACE)
target_compile_options (
    rayflow_warnings
    INTERFACE
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4244>" # int -> float conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4267>" # size_t -> int conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4305>" # double constant assigned to float
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4552>" # result of expression not used
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4838>" # double -> int conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4843>" # double -> float conversion
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd26451>" # arithmetic on 4-byte value, then cast to 8-byte
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd26495>" # uninitialized member variable
        "$<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CUDA>:SHELL:-Xcompiler >/wd4334>" # 32 to 64 bit displacement
)

include (CheckCXXSourceCompiles)

check_cxx_source_compiles ( "
#include <malloc.h>
int main() { void * ptr = _aligned_malloc(1024, 32); }
" HAVE_ALIGNED_MALLOC )

check_cxx_source_compiles ( "
#include <stdlib.h>
int main() {
  void *ptr;
  posix_memalign(&ptr, 32, 1024);
} " HAVE_POSIX_MEMALIGN )

if (HAVE_ALIGNED_MALLOC)
  list (APPEND RAYFLOW_MACRO_DEFINITIONS "RAYFLOW_HAVE_ALIGNED_MALLOC")
elseif (HAVE_POSIX_MEMALIGN)
  list (APPEND RAYFLOW_MACRO_DEFINITIONS "RAYFLOW_HAVE_POSIX_MEMALIGN")
else ()
  message (SEND_ERROR "Unable to find a way to allocate aligned memory")
endif ()

set (BUILD_SHARED_LIBS OFF)
add_subdirectory(ext)

set(RAYFLOW_INC_DIR ${PROJECT_SOURCE_DIR}/include/)
set(RAYFLOW_SRC_DIR ${PROJECT_SOURCE_DIR}/src/)
set(RAYFLOW_EXT_DIR ${PROJECT_SOURCE_DIR}/ext/)

include_directories(RayFlow
    ${RAYFLOW_EXT_DIR}
    ${RAYFLOW_INC_DIR}
    ${GLOG_INCLUDE_DIRS}
    ${STB_INCLUDE_DIRS}
    ${TINYXML2_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${TINYOBJLOADER_INCLUDE_DIRS}
)

set(RAYFLOW_ACCELERATE_SOURCES
    ${RAYFLOW_SRC_DIR}/Accelerate/bvh.cpp
) 

set(RAYFLOW_CORE_SOURCES
    ${RAYFLOW_SRC_DIR}/Core/integrator.cpp
    ${RAYFLOW_SRC_DIR}/Core/intersection.cpp
    ${RAYFLOW_SRC_DIR}/Core/spectrum.cpp
)

set(RAYFLOW_ENGINE_SOURCES
    ${RAYFLOW_SRC_DIR}/Engine/engine.cpp
    ${RAYFLOW_SRC_DIR}/Engine/memory_pool.cpp
    ${RAYFLOW_SRC_DIR}/Engine/parse.cpp
)

set(RAYFLOW_INTEGRATORS_SOURCES
    ${RAYFLOW_SRC_DIR}/Integrators/bdpt.cpp 
    ${RAYFLOW_SRC_DIR}/Integrators/direct.cpp
    ${RAYFLOW_SRC_DIR}/Integrators/pt.cpp
)

set(RAYFLOW_RENDER_SOURCES
    ${RAYFLOW_SRC_DIR}/Render/bxdfs.cpp
    ${RAYFLOW_SRC_DIR}/Render/cameras.cpp
    ${RAYFLOW_SRC_DIR}/Render/film.cpp
    ${RAYFLOW_SRC_DIR}/Render/light_sampler.cpp
    ${RAYFLOW_SRC_DIR}/Render/lights.cpp
    ${RAYFLOW_SRC_DIR}/Render/materials.cpp
    ${RAYFLOW_SRC_DIR}/Render/parallel.cpp
    ${RAYFLOW_SRC_DIR}/Render/samplers.cpp
    ${RAYFLOW_SRC_DIR}/Render/scene.cpp
    ${RAYFLOW_SRC_DIR}/Render/shapes.cpp
)

set(RAYFLOW_STD_SOURCES 
    ${RAYFLOW_SRC_DIR}/Std/memory_resource.cpp
)

set(RAYFLOW_UTIL_SOURCES
    ${RAYFLOW_SRC_DIR}/Util/bitmap.cpp
    ${RAYFLOW_SRC_DIR}/Util/half.cpp
    ${RAYFLOW_SRC_DIR}/Util/stb_image.cpp
)

add_executable(RayFlow 
    ${RAYFLOW_SRC_DIR}/main.cpp
    ${RAYFLOW_ACCELERATE_SOURCES}
    ${RAYFLOW_CORE_SOURCES}
    ${RAYFLOW_ENGINE_SOURCES}
    ${RAYFLOW_INTEGRATORS_SOURCES}
    ${RAYFLOW_RENDER_SOURCES}
    ${RAYFLOW_STD_SOURCES}
    ${RAYFLOW_UTIL_SOURCES}
)

target_link_libraries(RayFlow
    ${GLOG_LIBRARIES}
    ${TINYXML2_LIBRARIES}
    ${ASSIMP_LIBRARIES}
    ${TBB_LIBRARIES}
    ${TINYOBJLOADER_LIBRARIES}
)

target_compile_definitions(RayFlow PRIVATE ${RAYFLOW_MACRO_DEFINITIONS})
