/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-IEs"
 * 	found in "e2ap-ied-v03.01.asn"
 * 	`asn1c -pdu=auto -fincludes-quoted -fcompound-names -findirect-choice -fno-include-deps -no-gen-example -no-gen-OER -D /tmp/workspace/oransim-gerrit/e2sim/asn1c/`
 */

#ifndef	_E2nodeComponentInterfaceXn_H_
#define	_E2nodeComponentInterfaceXn_H_


#include "asn_application.h"

/* Including external dependencies */
#include "GlobalNG-RANNode-ID.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* E2nodeComponentInterfaceXn */
typedef struct E2nodeComponentInterfaceXn {
	GlobalNG_RANNode_ID_t	 global_NG_RAN_Node_ID;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} E2nodeComponentInterfaceXn_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_E2nodeComponentInterfaceXn;
extern asn_SEQUENCE_specifics_t asn_SPC_E2nodeComponentInterfaceXn_specs_1;
extern asn_TYPE_member_t asn_MBR_E2nodeComponentInterfaceXn_1[1];

#ifdef __cplusplus
}
#endif

#endif	/* _E2nodeComponentInterfaceXn_H_ */
#include "asn_internal.h"
