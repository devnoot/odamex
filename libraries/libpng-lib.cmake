### libpng (dep: zlib) ###

if(BUILD_CLIENT)
  find_package(PNG REQUIRED)
  if(TARGET PNG::PNG)
    set_target_properties(PNG::PNG PROPERTIES IMPORTED_GLOBAL True)
  endif()
endif()
