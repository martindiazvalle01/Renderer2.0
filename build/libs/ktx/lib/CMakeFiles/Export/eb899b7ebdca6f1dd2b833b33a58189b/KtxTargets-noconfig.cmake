#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "KTX::ktx" for configuration ""
set_property(TARGET KTX::ktx APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(KTX::ktx PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib64/libktx.so.5.0.0"
  IMPORTED_SONAME_NOCONFIG "libktx.so.5"
  )

list(APPEND _cmake_import_check_targets KTX::ktx )
list(APPEND _cmake_import_check_files_for_KTX::ktx "${_IMPORT_PREFIX}/lib64/libktx.so.5.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
