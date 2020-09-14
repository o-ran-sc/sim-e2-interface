# For clarity: this generates object, not a lib as the CM command implies.
#
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

include_directories(.)
add_library( asn1_objects OBJECT ${SOURCES})

target_include_directories (asn1_objects PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE src)

# header files should go into .../include/xfcpp/
if( DEV_PKG )                                   
  install( FILES
    ${HEADERS}
    DESTINATION ${install_inc}
    )
endif()
# ---------------------------------------------------------------------------
