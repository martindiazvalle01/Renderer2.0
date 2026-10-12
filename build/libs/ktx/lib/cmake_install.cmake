# Install script for directory: /home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/libs/ktx/lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/basisu-encoder/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/dfdutils/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/astc-encoder/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "library" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE SHARED_LIBRARY FILES
    "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/Debug/libktx.so.5.0.0"
    "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/Debug/libktx.so.5"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libktx.so.5.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libktx.so.5"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/llvm-strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/libs/ktx/lib/include/ktx.h"
    "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/libs/ktx/lib/include/ktxvulkan.h"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/KHR" TYPE FILE FILES "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/libs/ktx/lib/../external/dfdutils/KHR/khr_df.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "library" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE SHARED_LIBRARY FILES "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/Debug/libktx.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ktx/KtxTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ktx/KtxTargets.cmake"
         "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/CMakeFiles/Export/eb899b7ebdca6f1dd2b833b33a58189b/KtxTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ktx/KtxTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ktx/KtxTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ktx" TYPE FILE FILES "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/CMakeFiles/Export/eb899b7ebdca6f1dd2b833b33a58189b/KtxTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ktx" TYPE FILE FILES "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/CMakeFiles/Export/eb899b7ebdca6f1dd2b833b33a58189b/KtxTargets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ktx" TYPE FILE FILES
    "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/libs/ktx/lib/cmake/KtxConfig.cmake"
    "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/KtxConfigVersion.cmake"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/martindiaz/Documentos/ProyectosVulkan/Renderer2.0/build/libs/ktx/lib/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
