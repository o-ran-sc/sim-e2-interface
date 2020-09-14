# For clarity: this generates object, not a lib as the CM command implies.
#
add_library( def_objects OBJECT e2sim_defs.cpp)

target_include_directories (def_objects PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE src)

# header files should go into .../include/xfcpp/
if( DEV_PKG )
  install( FILES
    e2sim_defs.h
    DESTINATION ${install_inc}
    )
endif()
# ---------------------------------------------------------------------------
