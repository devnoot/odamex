### Protocol Buffers ###

if(BUILD_CLIENT OR BUILD_SERVER)
  find_package(protobuf CONFIG REQUIRED)
  set_target_properties(protobuf::libprotobuf PROPERTIES IMPORTED_GLOBAL True)
endif()
