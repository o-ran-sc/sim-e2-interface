# For clarity: this generates object, not a lib as the CM command implies.
#
add_library( messagerouting_objects OBJECT
         e2ap_message_handler.cpp
         e2ap_asn1c_codec.c
	 )
       
include_directories(../ASN1c)
include_directories(../encoding)       

target_include_directories (messagerouting_objects PUBLIC                    
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../ASN1c>  
  $<INSTALL_INTERFACE:include>
  PRIVATE src)

# header files should go into .../include/xfcpp/
if( DEV_PKG )                                                      
  install( FILES
    e2ap_message_handler.hpp
    DESTINATION ${install_inc}
    )
endif()
# ---------------------------------------------------------------------------
