/*****************************************************************************
#                                                                            *
# Copyright 2019 AT&T Intellectual Property                                  *
#                                                                            *
# Licensed under the Apache License, Version 2.0 (the "License");            *
# you may not use this file except in compliance with the License.           *
# You may obtain a copy of the License at                                    *
#                                                                            *
#      http://www.apache.org/licenses/LICENSE-2.0                            *
#                                                                            *
# Unless required by applicable law or agreed to in writing, software        *
# distributed under the License is distributed on an "AS IS" BASIS,          *
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
# See the License for the specific language governing permissions and        *
# limitations under the License.                                             *
#                                                                            *
******************************************************************************/

/*-
 * Copyright (c) 2003-2017 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#ifndef	ASN_SEQUENCE_OF_H
#define	ASN_SEQUENCE_OF_H

#include <asn_SET_OF.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SEQUENCE OF is the same as SET OF with a tiny difference:
 * the delete operation preserves the initial order of elements
 * and thus MAY operate in non-constant time.
 */
#define	A_SEQUENCE_OF(type)	A_SET_OF(type)

#define	ASN_SEQUENCE_ADD(headptr, ptr)		\
	asn_sequence_add((headptr), (ptr))

/***********************************************
 * Implementation of the SEQUENCE OF structure.
 */

#define	asn_sequence_add	asn_set_add
#define	asn_sequence_empty	asn_set_empty

/*
 * Delete the element from the set by its number (base 0).
 * This is NOT a constant-time operation.
 * The order of elements is preserved.
 * If _do_free is given AND the (*free) is initialized, the element
 * will be freed using the custom (*free) function as well.
 */
void asn_sequence_del(void *asn_sequence_of_x, int number, int _do_free);

/*
 * Cope with different conversions requirements to/from void in C and C++.
 * This is mostly useful for support library.
 */
typedef A_SEQUENCE_OF(void) asn_anonymous_sequence_;
#define _A_SEQUENCE_FROM_VOID(ptr)	((asn_anonymous_sequence_ *)(ptr))
#define _A_CSEQUENCE_FROM_VOID(ptr) 	((const asn_anonymous_sequence_ *)(ptr))

#ifdef __cplusplus
}
#endif

#endif	/* ASN_SEQUENCE_OF_H */
