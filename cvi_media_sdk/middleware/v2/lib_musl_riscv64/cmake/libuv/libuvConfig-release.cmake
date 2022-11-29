#----------------------------------------------------------------
# Generated CMake target import file for configuration "RELEASE".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "uv" for configuration "RELEASE"
set_property(TARGET uv APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(uv PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libuv.so.1.0.0"
  IMPORTED_SONAME_RELEASE "libuv.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS uv )
list(APPEND _IMPORT_CHECK_FILES_FOR_uv "${_IMPORT_PREFIX}/lib/libuv.so.1.0.0" )

# Import target "uv_a" for configuration "RELEASE"
set_property(TARGET uv_a APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(uv_a PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libuv_a.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS uv_a )
list(APPEND _IMPORT_CHECK_FILES_FOR_uv_a "${_IMPORT_PREFIX}/lib/libuv_a.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
