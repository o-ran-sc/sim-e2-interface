# For clarity: this generates object, not a lib as the CM command implies.
#
add_library( sctp_objects OBJECT e2sim_sctp.cpp e2sim_sctp.c)

target_include_directories (sctp_objects PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE src)

# header files should go into .../include/xfcpp/
if( DEV_PKG )
  install( FILES
    e2sim_sctp.hpp
    e2sim_sctp.h
    DESTINATION ${install_inc}
    )
endif()
# ---------------------------------------------------------------------------
