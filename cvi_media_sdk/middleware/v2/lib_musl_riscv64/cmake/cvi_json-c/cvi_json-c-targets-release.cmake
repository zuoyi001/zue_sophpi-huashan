#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cvi_json-c::cvi_json-c" for configuration "Release"
set_property(TARGET cvi_json-c::cvi_json-c APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cvi_json-c::cvi_json-c PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcvi_json-c.so.5.1.0"
  IMPORTED_SONAME_RELEASE "libcvi_json-c.so.5"
  )

list(APPEND _IMPORT_CHECK_TARGETS cvi_json-c::cvi_json-c )
list(APPEND _IMPORT_CHECK_FILES_FOR_cvi_json-c::cvi_json-c "${_IMPORT_PREFIX}/lib/libcvi_json-c.so.5.1.0" )

# Import target "cvi_json-c::cvi_json-c-static" for configuration "Release"
set_property(TARGET cvi_json-c::cvi_json-c-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cvi_json-c::cvi_json-c-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcvi_json-c.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cvi_json-c::cvi_json-c-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_cvi_json-c::cvi_json-c-static "${_IMPORT_PREFIX}/lib/libcvi_json-c.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
