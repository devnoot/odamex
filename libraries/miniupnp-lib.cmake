### MiniUPnPc ###

if(BUILD_SERVER AND USE_MINIUPNP AND USE_INTERNAL_LIBS)
  find_package(miniupnpc REQUIRED)
  set_target_properties(miniupnpc PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
