/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-IEs"
 * 	found in "/home/rshacham/e2ap-v01.00.00.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -fno-include-deps -findirect-choice -gen-PER -no-gen-OER -D .`
 */

#ifndef	_RICcontrolOutcome_H_
#define	_RICcontrolOutcome_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OCTET_STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RICcontrolOutcome */
typedef OCTET_STRING_t	 RICcontrolOutcome_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RICcontrolOutcome;
asn_struct_free_f RICcontrolOutcome_free;
asn_struct_print_f RICcontrolOutcome_print;
asn_constr_check_f RICcontrolOutcome_constraint;
ber_type_decoder_f RICcontrolOutcome_decode_ber;
der_type_encoder_f RICcontrolOutcome_encode_der;
xer_type_decoder_f RICcontrolOutcome_decode_xer;
xer_type_encoder_f RICcontrolOutcome_encode_xer;
per_type_decoder_f RICcontrolOutcome_decode_uper;
per_type_encoder_f RICcontrolOutcome_encode_uper;
per_type_decoder_f RICcontrolOutcome_decode_aper;
per_type_encoder_f RICcontrolOutcome_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _RICcontrolOutcome_H_ */
#include "asn_internal.h"
