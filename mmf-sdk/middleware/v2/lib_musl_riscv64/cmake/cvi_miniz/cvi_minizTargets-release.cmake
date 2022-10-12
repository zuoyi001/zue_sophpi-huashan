#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cvi_miniz::cvi_miniz" for configuration "Release"
set_property(TARGET cvi_miniz::cvi_miniz APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cvi_miniz::cvi_miniz PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcvi_miniz.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS cvi_miniz::cvi_miniz )
list(APPEND _IMPORT_CHECK_FILES_FOR_cvi_miniz::cvi_miniz "${_IMPORT_PREFIX}/lib/libcvi_miniz.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
