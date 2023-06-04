### JsonCpp ###

if(BUILD_SERVER)
  find_package(jsoncpp CONFIG REQUIRED)
  set_target_properties(jsoncpp_static PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
