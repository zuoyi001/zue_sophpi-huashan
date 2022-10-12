#----------------------------------------------------------------
# Generated CMake target import file for configuration "RELEASE".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "nanomsg" for configuration "RELEASE"
set_property(TARGET nanomsg APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(nanomsg PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libnanomsg.so.5.1.0"
  IMPORTED_SONAME_RELEASE "libnanomsg.so.5"
  )

list(APPEND _IMPORT_CHECK_TARGETS nanomsg )
list(APPEND _IMPORT_CHECK_FILES_FOR_nanomsg "${_IMPORT_PREFIX}/lib/libnanomsg.so.5.1.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
