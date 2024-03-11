#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "libcurl-simple-https" for configuration "Release"
set_property(TARGET libcurl-simple-https APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(libcurl-simple-https PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/liblibcurl-simple-https.a"
  )

list(APPEND _cmake_import_check_targets libcurl-simple-https )
list(APPEND _cmake_import_check_files_for_libcurl-simple-https "${_IMPORT_PREFIX}/lib/liblibcurl-simple-https.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
