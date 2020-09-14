# For clarity: this generates object, not a lib as the CM command implies.
#

add_library( base_objects OBJECT e2sim.cpp)

include_directories(../ASN1c)

target_include_directories (base_objects PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE src)

# header files should go into .../include/xfcpp/
if( DEV_PKG )                                   
  install( FILES
    e2sim.hpp
    DESTINATION ${install_inc}
    )
endif()
# ---------------------------------------------------------------------------
