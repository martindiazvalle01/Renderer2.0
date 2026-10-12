#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "meshoptimizer::meshoptimizer" for configuration ""
set_property(TARGET meshoptimizer::meshoptimizer APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(meshoptimizer::meshoptimizer PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib64/libmeshoptimizer.a"
  )

list(APPEND _cmake_import_check_targets meshoptimizer::meshoptimizer )
list(APPEND _cmake_import_check_files_for_meshoptimizer::meshoptimizer "${_IMPORT_PREFIX}/lib64/libmeshoptimizer.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
