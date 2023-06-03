### zlib ###

if(BUILD_CLIENT OR BUILD_SERVER)
  find_package(ZLIB REQUIRED)
  if(TARGET ZLIB::ZLIB)
    set_target_properties(ZLIB::ZLIB PROPERTIES IMPORTED_GLOBAL True)
  endif()
endif()
