/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-PDU-Contents"
 * 	found in "/home/rshacham/e2ap-v01.00.00.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -fno-include-deps -findirect-choice -gen-PER -no-gen-OER -D .`
 */

#ifndef	_RICsubscriptionResponse_H_
#define	_RICsubscriptionResponse_H_


#include "asn_application.h"

/* Including external dependencies */
#include "ProtocolIE-Container.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RICsubscriptionResponse */
typedef struct RICsubscriptionResponse {
	ProtocolIE_Container_1527P1_t	 protocolIEs;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RICsubscriptionResponse_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RICsubscriptionResponse;
extern asn_SEQUENCE_specifics_t asn_SPC_RICsubscriptionResponse_specs_1;
extern asn_TYPE_member_t asn_MBR_RICsubscriptionResponse_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _RICsubscriptionResponse_H_ */
#include "asn_internal.h"
