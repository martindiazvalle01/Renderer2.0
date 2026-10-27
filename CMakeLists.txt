cmake_minimum_required(VERSION 3.25)
project(VulkanEngine LANGUAGES CXX C)

# =============================================================================
# CONFIGURACIÓN BASE
# =============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directorio de dependencias (ajústalo a tu estructura)
set(LIBRARY_DIR "${CMAKE_SOURCE_DIR}/libs")

# =============================================================================
# SDL3
# =============================================================================
#  SDL3 preinstalado (vcpkg, system package)
find_package(SDL3 REQUIRED CONFIG)


# =============================================================================
# VULKAN
# =============================================================================
find_package(Vulkan REQUIRED)

# =============================================================================
# GLM (header-only)
# =============================================================================
find_package(glm QUIET)
if(NOT glm_FOUND)
    add_subdirectory(${LIBRARY_DIR}/glm)
endif()

# =============================================================================
# SPDLOG
# =============================================================================
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(${LIBRARY_DIR}/spdlog)

# =============================================================================
# TRACY (profiler)
# =============================================================================
set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
set(TRACY_CALLSTACK 10 CACHE STRING "" FORCE)
add_subdirectory(${LIBRARY_DIR}/tracy)


# =============================================================================
# VMA (Vulkan Memory Allocator)
# =============================================================================
add_library(vma INTERFACE)
target_include_directories(vma INTERFACE ${LIBRARY_DIR}/vma/include)

# =============================================================================
# MESHOPTIMIZER
# =============================================================================
set(MESHOPTIMIZER_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(${LIBRARY_DIR}/meshoptimizer)



# =============================================================================
# KTX-SOFTWARE
# =============================================================================
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_DOC OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_STATIC_LIBRARY ON CACHE BOOL "" FORCE)
add_subdirectory(${LIBRARY_DIR}/ktx)

# =============================================================================
# SLANG (shader compiler)
# =============================================================================
# Slang no tiene un CMakeLists.txt limpio. Lo linkeamos manualmente.
# Asume que tienes los headers y la lib precompilada en third_party/slang/
add_library(slang INTERFACE)
target_include_directories(slang INTERFACE ${LIBRARY_DIR}/slang/include)
target_link_directories(slang INTERFACE ${LIBRARY_DIR}/slang/lib)
target_link_libraries(slang INTERFACE slang)

# =============================================================================
# IMGUI + IMGUIZMO
# =============================================================================
set(IMGUI_DIR ${LIBRARY_DIR}/imgui)
set(IMGUIZMO_DIR ${LIBRARY_DIR}/imguizmo)


# =============================================================================
# FASTGLTF
# =============================================================================
add_subdirectory(${LIBRARY_DIR}/fastgltf)



file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

file(GLOB IMGUIZMO_SOURCES
    ${IMGUIZMO_DIR}/ImGuizmo.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUIZMO_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUIZMO_DIR}
)
target_link_libraries(imgui PUBLIC SDL3::SDL3 Vulkan::Vulkan)

# =============================================================================
# MOTOR
# =============================================================================

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Enlazando carpeta de assets al directorio de build"
)

target_include_directories(${PROJECT_NAME}  SYSTEM PRIVATE
    "src"
    "src/core_layers/Single_GPU_Core"
    "src/resource_management_layer"
    "src/resource_management_layer/GLTF"
    "src/resource_management_layer/Single_GPU_Manager"
    "src/resource_management_layer/texture"
    "src/resource_management_layer/Mesh"
    "src/Tests"
    "src/rendering_layers"
    "src/rendering_layers/swapchain"
    "src/rendering_layers/FrameManagement"
    "src/rendering_layers/Renderer"
    "libs/vma/include"
    "libs/tinygltf"
    ${STB_DIR}

    
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
    vma
    glm::glm
    spdlog::spdlog
    Tracy::TracyClient
    meshoptimizer
    ktx
    slang
    imgui
    fastgltf::fastgltf
)

# =============================================================================
# FLAGS DEL COMPILADOR (C++23 + optimizaciones)
# =============================================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        /Zc:preprocessor
    )
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-missing-field-initializers
    )
endif()

# =============================================================================
# SHADERS (compilación automática con Slang)
# =============================================================================
file(GLOB_RECURSE SLANG_SHADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/assets/shaders/*.slang)

find_program(SLANGC slangc PATHS ${LIBRARY_DIR}/slang/bin)
if(NOT SLANGC)
    message(FATAL_ERROR "No se encontro slangc (buscado en ${LIBRARY_DIR}/slang/bin y en el PATH)")
endif()

set(SPIRV_OUTPUTS "")
foreach(shader ${SLANG_SHADERS})
    get_filename_component(dir  ${shader} DIRECTORY)
    get_filename_component(name ${shader} NAME_WE)
    set(spv ${dir}/${name}.spv)

    add_custom_command(
        OUTPUT ${spv}
        COMMAND ${SLANGC} ${shader}
            -target spirv
            -profile spirv_1_4
            -fvk-use-entrypoint-name
            -o ${spv}
        DEPENDS ${shader}
        COMMENT "Compilando shader: ${name}.slang"
        VERBATIM
    )
    list(APPEND SPIRV_OUTPUTS ${spv})
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(${PROJECT_NAME} shaders)