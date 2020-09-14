# For clarity: this generates object, not a lib as the CM command implies.
#
add_library( encoding_objects OBJECT encode_e2apv1.cpp)

include_directories(../ASN1c)

target_include_directories (encoding_objects PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE src )


# header files should go into .../include/xfcpp/
if( DEV_PKG )
  install( FILES
    encode_e2apv1.hpp
    DESTINATION ${install_inc}
    )
endif()
# ---------------------------------------------------------------------------
