#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "rex::runtime" for configuration "Debug"
set_property(TARGET rex::runtime APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::runtime PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/rexruntimed.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/rexruntimed.dll"
  )

list(APPEND _cmake_import_check_targets rex::runtime )
list(APPEND _cmake_import_check_files_for_rex::runtime "${_IMPORT_PREFIX}/lib/rexruntimed.lib" "${_IMPORT_PREFIX}/bin/rexruntimed.dll" )

# Import target "rex::gpu-xenos" for configuration "Debug"
set_property(TARGET rex::gpu-xenos APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::gpu-xenos PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/rexgpu-xenosd.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG "rex::runtime"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/rexgpu-xenosd.dll"
  )

list(APPEND _cmake_import_check_targets rex::gpu-xenos )
list(APPEND _cmake_import_check_files_for_rex::gpu-xenos "${_IMPORT_PREFIX}/lib/rexgpu-xenosd.lib" "${_IMPORT_PREFIX}/bin/rexgpu-xenosd.dll" )

# Import target "rex::aes128" for configuration "Debug"
set_property(TARGET rex::aes128 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::aes128 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/aes128d.lib"
  )

list(APPEND _cmake_import_check_targets rex::aes128 )
list(APPEND _cmake_import_check_files_for_rex::aes128 "${_IMPORT_PREFIX}/lib/aes128d.lib" )

# Import target "rex::mspack" for configuration "Debug"
set_property(TARGET rex::mspack APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::mspack PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/mspackd.lib"
  )

list(APPEND _cmake_import_check_targets rex::mspack )
list(APPEND _cmake_import_check_files_for_rex::mspack "${_IMPORT_PREFIX}/lib/mspackd.lib" )

# Import target "rex::o1heap" for configuration "Debug"
set_property(TARGET rex::o1heap APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::o1heap PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/o1heapd.lib"
  )

list(APPEND _cmake_import_check_targets rex::o1heap )
list(APPEND _cmake_import_check_files_for_rex::o1heap "${_IMPORT_PREFIX}/lib/o1heapd.lib" )

# Import target "rex::disasm" for configuration "Debug"
set_property(TARGET rex::disasm APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::disasm PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/disasmd.lib"
  )

list(APPEND _cmake_import_check_targets rex::disasm )
list(APPEND _cmake_import_check_files_for_rex::disasm "${_IMPORT_PREFIX}/lib/disasmd.lib" )

# Import target "rex::xxhash" for configuration "Debug"
set_property(TARGET rex::xxhash APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::xxhash PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/xxhashd.lib"
  )

list(APPEND _cmake_import_check_targets rex::xxhash )
list(APPEND _cmake_import_check_files_for_rex::xxhash "${_IMPORT_PREFIX}/lib/xxhashd.lib" )

# Import target "rex::libavcodec" for configuration "Debug"
set_property(TARGET rex::libavcodec APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::libavcodec PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libavcodecd.lib"
  )

list(APPEND _cmake_import_check_targets rex::libavcodec )
list(APPEND _cmake_import_check_files_for_rex::libavcodec "${_IMPORT_PREFIX}/lib/libavcodecd.lib" )

# Import target "rex::libavutil" for configuration "Debug"
set_property(TARGET rex::libavutil APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::libavutil PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libavutild.lib"
  )

list(APPEND _cmake_import_check_targets rex::libavutil )
list(APPEND _cmake_import_check_files_for_rex::libavutil "${_IMPORT_PREFIX}/lib/libavutild.lib" )

# Import target "rex::TracyClient" for configuration "Debug"
set_property(TARGET rex::TracyClient APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(rex::TracyClient PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/TracyClientd.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/TracyClientd.dll"
  )

list(APPEND _cmake_import_check_targets rex::TracyClient )
list(APPEND _cmake_import_check_files_for_rex::TracyClient "${_IMPORT_PREFIX}/lib/TracyClientd.lib" "${_IMPORT_PREFIX}/bin/TracyClientd.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
