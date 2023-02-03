

/*****************************************************************************
#                                                                            *
# Copyright 2020 AT&T Intellectual Property                                  *
# Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved.      *
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

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include <iterator>
#include <vector>


#include "encode_e2apv1.hpp"

extern "C" {


#include "e2ap_asn1c_codec.h"
#include "GlobalE2node-ID.h"
#include "GlobalE2node-gNB-ID.h"
#include "GlobalgNB-ID.h"
#include "OCTET_STRING.h"
#include "asn_application.h"
#include "GNB-ID-Choice.h"
#include "ProtocolIE-Field.h"
#include "E2setupRequest.h"
#include "RICaction-ToBeSetup-Item.h"
#include "RICactions-ToBeSetup-List.h"
#include "RICeventTriggerDefinition.h"
#include "RICsubscriptionRequest.h"
#include "RICsubscriptionResponse.h"
#include "ProtocolIE-SingleContainer.h"
#include "RANfunctions-List.h"
#include "RICindication.h"
#include "RICsubsequentActionType.h"
#include "RICsubsequentAction.h"  
#include "RICtimeToWait.h"
#include "E2nodeComponentInterfaceNG.h"
// #include "E2nodeComponentGNB-CU-UP-ID.h"
// #include "E2nodeComponentID.h"
// #include "E2nodeComponentConfigUpdate.h"
// #include "E2nodeComponentConfigUpdateGNB.h"
  
}

long encoding::get_function_id_from_subscription(E2AP_PDU_t *e2ap_pdu) {

  RICsubscriptionRequest_t orig_req =
    e2ap_pdu->choice.initiatingMessage->value.choice.RICsubscriptionRequest;

  int count = orig_req.protocolIEs.list.count;
  int size = orig_req.protocolIEs.list.size;
  
  RICsubscriptionRequest_IEs_t **ies = (RICsubscriptionRequest_IEs_t**)orig_req.protocolIEs.list.array;

  fprintf(stderr, "count%d\n", count);
  fprintf(stderr, "size%d\n", size);

  RICsubscriptionRequest_IEs__value_PR pres;

  long func_id;

  for (int i=0; i < count; i++) {
    RICsubscriptionRequest_IEs_t *next_ie = ies[i];
    pres = next_ie->value.present;
    
    fprintf(stderr, "next present value %d\n", pres);
    fprintf(stderr, "value of pres ranfuncid is %d\n", RICsubscriptionRequest_IEs__value_PR_RANfunctionID);

    if (pres == RICsubscriptionRequest_IEs__value_PR_RANfunctionID) {
      fprintf(stderr, "equal pres to ranfuncid\n");
      func_id = next_ie->value.choice.RANfunctionID;
    }
    
  }

  fprintf(stderr, "After loop, func_id is %d\n", func_id);

  return func_id;  

}

void encoding::generate_e2apv1_service_update(E2AP_PDU_t *e2ap_pdu, std::vector<encoding::ran_func_info> all_funcs) {
  char* ran_function_op_type = getenv("RAN_FUNCTION_OP_TYPE");
  LOG_D("Ran funciton : %s", ran_function_op_type);
  ProtocolIE_ID_t prID;
  if (ran_function_op_type != NULL)
  {
    if (strcmp(ran_function_op_type, "ADD") == 0)
    {
      prID = ProtocolIE_ID_id_RANfunctionsAdded;
    }
    else if (strcmp(ran_function_op_type, "DELETE"))
    {
      prID = ProtocolIE_ID_id_RANfunctionsDeleted;
    }
  }
  else
  {
    prID = ProtocolIE_ID_id_RANfunctionsModified;
  }


  RICserviceUpdate_IEs_t *e2serviceUpdateList = (RICserviceUpdate_IEs_t *)calloc(1, sizeof(RICserviceUpdate_IEs_t));
  e2serviceUpdateList->id = prID;
  e2serviceUpdateList->criticality = Criticality_reject;
  e2serviceUpdateList->value.present = RICserviceUpdate_IEs__value_PR_RANfunctions_List;


  for (int i=0; i<all_funcs.size(); i++) {

    encoding::ran_func_info nextRanFunc = all_funcs.at(i);
    long nextRanFuncId = nextRanFunc.ranFunctionId;
    OCTET_STRING_t *nextRanFuncDesc = nextRanFunc.ranFunctionDesc;
    long nextRanFuncRev = nextRanFunc.ranFunctionRev;

    auto *itemIes = (RANfunction_ItemIEs_t *)calloc(1, sizeof(RANfunction_ItemIEs_t));
    itemIes->id = ProtocolIE_ID_id_RANfunction_Item;
    itemIes->criticality = Criticality_reject;
    itemIes->value.present = RANfunction_ItemIEs__value_PR_RANfunction_Item;
    itemIes->value.choice.RANfunction_Item.ranFunctionID = 1;

    itemIes->value.choice.RANfunction_Item.ranFunctionDefinition = *nextRanFuncDesc;
    itemIes->value.choice.RANfunction_Item.ranFunctionRevision = nextRanFuncRev + 1;

    ASN_SEQUENCE_ADD(&e2serviceUpdateList->value.choice.RANfunctions_List.list, itemIes);
  }

  RICserviceUpdate_t *ricServiceUpdate = (RICserviceUpdate_t *)calloc(1, sizeof(RICserviceUpdate_t));
  ASN_SEQUENCE_ADD(&ricServiceUpdate->protocolIEs.list, e2serviceUpdateList);

  InitiatingMessage_t *initiatingMessage = (InitiatingMessage_t *)calloc(1, sizeof(InitiatingMessage_t));
  initiatingMessage->criticality = Criticality_reject;
  initiatingMessage->procedureCode = ProcedureCode_id_RICserviceUpdate;
  initiatingMessage->value.present = InitiatingMessage__value_PR_RICserviceUpdate;
  initiatingMessage->value.choice.RICserviceUpdate = *ricServiceUpdate;

  E2AP_PDU_PR pres6 = E2AP_PDU_PR_initiatingMessage;
  e2ap_pdu->present = pres6;
  e2ap_pdu->choice.initiatingMessage = initiatingMessage;
}

void encoding::generate_e2apv1_setup_request_parameterized(E2AP_PDU_t *e2ap_pdu, std::vector<ran_func_info> all_funcs) {

  BIT_STRING_t *gnb_bstring = (BIT_STRING_t*)calloc(1, sizeof(BIT_STRING_t));;
  gnb_bstring->buf = (uint8_t*)calloc(1,4);
  gnb_bstring->size = 4;
  gnb_bstring->buf[0] = 0xB5;
  gnb_bstring->buf[1] = 0xC6;
  gnb_bstring->buf[2] = 0x77;
  gnb_bstring->buf[3] = 0x88;

  gnb_bstring->bits_unused = 3;

  uint8_t *buf2 = (uint8_t *)"747";
  OCTET_STRING_t *plmn = (OCTET_STRING_t*)calloc(1, sizeof(OCTET_STRING_t));
  plmn->buf = (uint8_t*)calloc(1,3);
  memcpy(plmn->buf, buf2, 3);
  plmn->size = 3;

  GNB_ID_Choice_t *gnbchoice = (GNB_ID_Choice_t*)calloc(1,sizeof(GNB_ID_Choice_t));
  GNB_ID_Choice_PR pres2 = GNB_ID_Choice_PR_gnb_ID;
  gnbchoice->present = pres2;
  gnbchoice->choice.gnb_ID = *gnb_bstring;
  if (gnb_bstring) free(gnb_bstring);

  GlobalgNB_ID_t *gnb = (GlobalgNB_ID_t*)calloc(1, sizeof(GlobalgNB_ID_t));
  gnb->plmn_id = *plmn;
  gnb->gnb_id = *gnbchoice;
  if (plmn) free(plmn);
  if (gnbchoice) free(gnbchoice);

  GlobalE2node_gNB_ID_t *e2gnb = (GlobalE2node_gNB_ID_t*)calloc(1, sizeof(GlobalE2node_gNB_ID_t));
  e2gnb->global_gNB_ID = *gnb;
  if (gnb) free(gnb);

  GlobalE2node_ID_t *globale2nodeid = (GlobalE2node_ID_t*)calloc(1, sizeof(GlobalE2node_ID_t));
  GlobalE2node_ID_PR pres;
  pres = GlobalE2node_ID_PR_gNB;
  globale2nodeid->present = pres;
  globale2nodeid->choice.gNB = e2gnb;
  
  E2setupRequestIEs_t *e2setuprid = (E2setupRequestIEs_t*)calloc(1, sizeof(E2setupRequestIEs_t));
  E2setupRequestIEs__value_PR pres3;
  pres3 = E2setupRequestIEs__value_PR_GlobalE2node_ID;
  e2setuprid->id = 3;
  e2setuprid->criticality = 0;
  e2setuprid->value.choice.GlobalE2node_ID = *globale2nodeid;
  e2setuprid->value.present = pres3;
  if(globale2nodeid) free(globale2nodeid);


  //seting tx id
  auto *e2txid = (E2setupRequestIEs_t *)calloc(1, sizeof(E2setupRequestIEs_t));
  e2txid->id = ProtocolIE_ID_id_TransactionID;
  e2txid-> criticality = 0;
  e2txid->value.present = E2setupRequestIEs__value_PR_TransactionID;
  e2txid->value.choice.TransactionID = 1;

  auto *ranFlistIEs = (E2setupRequestIEs_t *)calloc(1, sizeof(E2setupRequestIEs_t));
  ASN_STRUCT_RESET(asn_DEF_E2setupRequestIEs, ranFlistIEs);
  ranFlistIEs->criticality = 0;
  ranFlistIEs->id = ProtocolIE_ID_id_RANfunctionsAdded;
  ranFlistIEs->value.present = E2setupRequestIEs__value_PR_RANfunctions_List;


  for (int i = 0; i < all_funcs.size(); i++) {

    ran_func_info nextRanFunc = all_funcs.at(i);
    long nextRanFuncId = nextRanFunc.ranFunctionId;
    OCTET_STRING_t *nextRanFuncDesc = nextRanFunc.ranFunctionDesc;
    long nextRanFuncRev = nextRanFunc.ranFunctionRev;

    auto *itemIes = (RANfunction_ItemIEs_t *)calloc(1, sizeof(RANfunction_ItemIEs_t));
    itemIes->id = ProtocolIE_ID_id_RANfunction_Item;
    itemIes->criticality = Criticality_reject;
    itemIes->value.present = RANfunction_ItemIEs__value_PR_RANfunction_Item;
    itemIes->value.choice.RANfunction_Item.ranFunctionID = nextRanFuncId;
    itemIes->value.choice.RANfunction_Item.ranFunctionOID = RANfunctionOID_t(*(nextRanFunc.ranFunctionOId));
    int ranFuncLength = strlen((char*)nextRanFuncDesc);

    itemIes->value.choice.RANfunction_Item.ranFunctionDefinition = *nextRanFuncDesc;
    itemIes->value.choice.RANfunction_Item.ranFunctionRevision = nextRanFuncRev;
    
    ASN_SEQUENCE_ADD(&ranFlistIEs->value.choice.RANfunctions_List.list, itemIes);

  }

auto *e2configIE = (E2setupRequestIEs_t *)calloc(1, sizeof(E2setupRequestIEs_t));
e2configIE->id = ProtocolIE_ID_id_E2nodeComponentConfigAddition;
e2configIE->criticality = Criticality_reject;
e2configIE->value.present = E2setupRequestIEs__value_PR_E2nodeComponentConfigAddition_List;


auto *e2configAdditionItem = (E2nodeComponentConfigAddition_ItemIEs_t *)calloc(1, sizeof(E2nodeComponentConfigAddition_ItemIEs_t));
e2configAdditionItem->id = ProtocolIE_ID_id_E2nodeComponentConfigAddition_Item;
e2configAdditionItem->criticality = Criticality_reject;
e2configAdditionItem->value.present = E2nodeComponentConfigAddition_ItemIEs__value_PR_E2nodeComponentConfigAddition_Item;

e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentInterfaceType = E2nodeComponentInterfaceType_ng;
e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentID.present = E2nodeComponentID_PR_e2nodeComponentInterfaceTypeNG;

auto *intfNG = (E2nodeComponentInterfaceNG_t *) calloc(1, sizeof(E2nodeComponentInterfaceNG_t));
  
OCTET_STRING_t nginterf;
nginterf.buf = (uint8_t*)calloc(1,8);
memcpy(nginterf.buf, (uint8_t *)"nginterf", 8);

nginterf.size = 8;
intfNG->amf_name = (AMFName_t)(nginterf);

e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentID.choice.e2nodeComponentInterfaceTypeNG = intfNG;

OCTET_STRING_t reqPart;
reqPart.buf = (uint8_t*)calloc(1,7);
memcpy(reqPart.buf, (uint8_t *)"reqpart", 7);
reqPart.size = 7;

e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentConfiguration.e2nodeComponentRequestPart = reqPart;

OCTET_STRING_t resPart;
resPart.buf = (uint8_t*)calloc(1,7);
memcpy(resPart.buf, (uint8_t *)"respart", 7);
resPart.size = 7;
e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentConfiguration.e2nodeComponentResponsePart = resPart;


ASN_SEQUENCE_ADD(&e2configIE->value.choice.RANfunctions_List.list, e2configAdditionItem);
/*
    auto *e2nodeconfigupdatelistIEs = (E2setupRequestIEs_t *)calloc(1, sizeof(E2setupRequestIEs_t));
  ASN_STRUCT_RESET(asn_DEF_E2setupRequestIEs, e2nodeconfigupdatelistIEs);
  e2nodeconfigupdatelistIEs->criticality = 0;
  e2nodeconfigupdatelistIEs->id = ProtocolIE_ID_id_E2nodeComponentConfigUpdate;
  e2nodeconfigupdatelistIEs->value.present = E2setupRequestIEs__value_PR_E2nodeComponentConfigUpdate_List;


  OCTET_STRING_t *ngAPconfigUpdate =  (OCTET_STRING_t *)calloc(1,sizeof(OCTET_STRING_t));
  ngAPconfigUpdate->buf = (uint8_t*)calloc(1,13);
  memcpy(ngAPconfigUpdate->buf, (uint8_t *)"gnbngapupdate", 13);
  ngAPconfigUpdate->size = 13;

  OCTET_STRING_t *xnAPconfigUpdate =  (OCTET_STRING_t *)calloc(1,sizeof(OCTET_STRING_t));
  xnAPconfigUpdate->buf = (uint8_t*)calloc(1,13);
  memcpy(xnAPconfigUpdate->buf, (uint8_t *)"gnbxnapupdate", 13);
  xnAPconfigUpdate->size = 13;

  OCTET_STRING_t *e1APconfigUpdate =  (OCTET_STRING_t *)calloc(1,sizeof(OCTET_STRING_t));
  e1APconfigUpdate->buf = (uint8_t*)calloc(1,13);
  memcpy(e1APconfigUpdate->buf, (uint8_t *)"gnbe1apupdate", 13);
  e1APconfigUpdate->size = 13;

  OCTET_STRING_t *f1APconfigUpdate =  (OCTET_STRING_t *)calloc(1,sizeof(OCTET_STRING_t));
  f1APconfigUpdate->buf = (uint8_t*)calloc(1,13);
  memcpy(f1APconfigUpdate->buf, (uint8_t *)"gnbf1apupdate", 13);
  f1APconfigUpdate->size = 13;

  E2nodeComponentConfigUpdateGNB_t *e2nodecomponentconfigupdategnb = (E2nodeComponentConfigUpdateGNB_t *)calloc(1,sizeof(E2nodeComponentConfigUpdateGNB_t));
  e2nodecomponentconfigupdategnb->ngAPconfigUpdate = ngAPconfigUpdate;
  e2nodecomponentconfigupdategnb->xnAPconfigUpdate = xnAPconfigUpdate;
  e2nodecomponentconfigupdategnb->e1APconfigUpdate = e1APconfigUpdate;
  e2nodecomponentconfigupdategnb->f1APconfigUpdate = f1APconfigUpdate;

  E2nodeComponentConfigUpdate_t e2nodecomponentconfigupdate = {};
  e2nodecomponentconfigupdate.present = E2nodeComponentConfigUpdate_PR_gNBconfigUpdate;
  e2nodecomponentconfigupdate.choice.gNBconfigUpdate = e2nodecomponentconfigupdategnb;

  GNB_CU_UP_ID_t gnbcuupid = {};
  gnbcuupid.buf = (uint8_t*)calloc(1,4);
  memcpy(gnbcuupid.buf, (uint8_t *)"1234", 4);
  gnbcuupid.size = 4;

  E2nodeComponentGNB_CU_UP_ID_t *e2nodecomponentgnbcuupid = (E2nodeComponentGNB_CU_UP_ID_t *)calloc(1,sizeof(E2nodeComponentGNB_CU_UP_ID_t));
  e2nodecomponentgnbcuupid->gNB_CU_UP_ID = gnbcuupid;

  E2nodeComponentID_t *e2nodecomponentid = (E2nodeComponentID_t*)calloc(1, sizeof(E2nodeComponentID_t));

  e2nodecomponentid->present = E2nodeComponentID_PR_e2nodeComponentTypeGNB_CU_UP;
  e2nodecomponentid->choice.e2nodeComponentTypeGNB_CU_UP = e2nodecomponentgnbcuupid;

  auto *configupdateitemIes = (E2nodeComponentConfigUpdate_ItemIEs_t *)calloc(1, sizeof(E2nodeComponentConfigUpdate_ItemIEs_t));
  configupdateitemIes->id = ProtocolIE_ID_id_E2nodeComponentConfigUpdate_Item;
  configupdateitemIes->criticality = Criticality_reject;
  configupdateitemIes->value.present = E2nodeComponentConfigUpdate_ItemIEs__value_PR_E2nodeComponentConfigUpdate_Item;
  configupdateitemIes->value.choice.E2nodeComponentConfigUpdate_Item.e2nodeComponentType = E2nodeComponentType_gNB_CU_UP;
  configupdateitemIes->value.choice.E2nodeComponentConfigUpdate_Item.e2nodeComponentID = e2nodecomponentid;
  configupdateitemIes->value.choice.E2nodeComponentConfigUpdate_Item.e2nodeComponentConfigUpdate = e2nodecomponentconfigupdate;

  ASN_SEQUENCE_ADD(&e2nodeconfigupdatelistIEs->value.choice.E2nodeComponentConfigUpdate_List.list, configupdateitemIes);
*/
  E2setupRequest_t *e2setupreq = (E2setupRequest_t*)calloc(1, sizeof(E2setupRequest_t));
  ASN_SEQUENCE_ADD(&e2setupreq->protocolIEs.list, e2txid);
  ASN_SEQUENCE_ADD(&e2setupreq->protocolIEs.list, e2setuprid);
  ASN_SEQUENCE_ADD(&e2setupreq->protocolIEs.list, ranFlistIEs);
  ASN_SEQUENCE_ADD(&e2setupreq->protocolIEs.list, e2configIE);


  InitiatingMessage__value_PR pres4;
  pres4 = InitiatingMessage__value_PR_E2setupRequest;
  InitiatingMessage_t *initmsg = (InitiatingMessage_t*)calloc(1, sizeof(InitiatingMessage_t));

  initmsg->procedureCode = ProcedureCode_id_E2setup;
  initmsg->criticality = Criticality_reject;
  initmsg->value.present = pres4;
  initmsg->value.choice.E2setupRequest = *e2setupreq;
  if (e2setupreq) free(e2setupreq);

  E2AP_PDU_PR pres5;
  pres5 = E2AP_PDU_PR_initiatingMessage;
  

  e2ap_pdu->present = pres5;
  e2ap_pdu->choice.initiatingMessage = initmsg;  

}

void encoding::generate_e2apv2_reset_request(E2AP_PDU *e2ap_pdu) {
  E2AP_PDU_PR pr = E2AP_PDU_PR_initiatingMessage;
  e2ap_pdu->present = pr;

  e2ap_pdu->choice.initiatingMessage = (InitiatingMessage_t*)calloc(1, sizeof(InitiatingMessage_t));
  e2ap_pdu->choice.initiatingMessage->procedureCode = ProcedureCode_id_Reset;
  e2ap_pdu->choice.initiatingMessage->criticality = Criticality_reject;

  e2ap_pdu->choice.initiatingMessage->value.present = InitiatingMessage__value_PR_ResetRequest;

  auto *rrIEs1 = (ResetRequestIEs_t *)calloc(1, sizeof(ResetRequestIEs_t));
  rrIEs1->id = ProtocolIE_ID_id_Cause;
  rrIEs1->criticality = Criticality_ignore;

  rrIEs1->value.present = ResetRequestIEs__value_PR_Cause;
  rrIEs1->value.choice.Cause.present = Cause_PR_e2Node;
  rrIEs1->value.choice.Cause.choice.ricRequest = CauseE2node_e2node_component_unknown;

  auto *rrIEs2 = (ResetRequestIEs_t *)calloc(1, sizeof(ResetRequestIEs_t));
  rrIEs2->id = ProtocolIE_ID_id_TransactionID;
  rrIEs2->criticality = Criticality_ignore;
  
  rrIEs2->value.present = ResetRequestIEs__value_PR_TransactionID;
  rrIEs2->value.choice.TransactionID = 1;

  ASN_SEQUENCE_ADD(&e2ap_pdu->choice.initiatingMessage->value.choice.ResetRequest.protocolIEs.list, rrIEs2);
  ASN_SEQUENCE_ADD(&e2ap_pdu->choice.initiatingMessage->value.choice.ResetRequest.protocolIEs.list, rrIEs1);
}

void encoding::generate_e2apv2_reset_response(E2AP_PDU *e2ap_pdu) {
  e2ap_pdu->present = E2AP_PDU_PR_successfulOutcome;
  e2ap_pdu->choice.successfulOutcome = (SuccessfulOutcome_t*)calloc(1, sizeof(SuccessfulOutcome));

  e2ap_pdu->choice.successfulOutcome->procedureCode = ProcedureCode_id_Reset;
  e2ap_pdu->choice.successfulOutcome->criticality = Criticality_ignore;

  e2ap_pdu->choice.successfulOutcome->value.present = SuccessfulOutcome__value_PR_ResetResponse;

  // auto *rrIEs1 = (ResetResponseIEs_t *)calloc(1, sizeof(ResetResponseIEs_t));
  // rrIEs1->id = ProtocolIE_ID_id_CriticalityDiagnostics;
  // rrIEs1->criticality = Criticality_ignore;

  // rrIEs1->value.present = ResetResponseIEs__value_PR_CriticalityDiagnostics;
  // rrIEs1->value.choice.CriticalityDiagnostics.procedureCode = &ProcedureCode_id_Reset;
  // rrIEs1->value.choice.CriticalityDiagnostics.choice.ricRequest = CauseE2node_e2node_component_unknown;

  auto *rrIEs2 = (ResetRequestIEs_t *)calloc(1, sizeof(ResetRequestIEs_t));
  rrIEs2->id = ProtocolIE_ID_id_TransactionID;
  rrIEs2->criticality = Criticality_ignore;
  
  rrIEs2->value.present = ResetRequestIEs__value_PR_TransactionID;
  rrIEs2->value.choice.TransactionID = 1;

  ASN_SEQUENCE_ADD(&e2ap_pdu->choice.successfulOutcome->value.choice.ResetResponse.protocolIEs.list, rrIEs2);
}

void encoding::generate_e2apv2_config_update(E2AP_PDU_t *e2ap_pdu){
 // txid
 auto *e2txidIE = (E2nodeConfigurationUpdate_IEs_t *)calloc(1, sizeof(E2nodeConfigurationUpdate_IEs_t));
 e2txidIE->id = ProtocolIE_ID_id_TransactionID;
 e2txidIE-> criticality = Criticality_reject;
 e2txidIE->value.present = E2nodeConfigurationUpdate_IEs__value_PR_TransactionID;
 e2txidIE->value.choice.TransactionID = 1;
  
/// config update id for addtion list  
auto *e2configIE = (E2nodeConfigurationUpdate_IEs_t *)calloc(1, sizeof(E2nodeConfigurationUpdate_IEs_t));
e2configIE->id = ProtocolIE_ID_id_E2nodeComponentConfigAddition;
e2configIE->criticality = Criticality_reject;
e2configIE->value.present = E2nodeConfigurationUpdate_IEs__value_PR_E2nodeComponentConfigAddition_List;


auto *e2configAdditionItem = (E2nodeComponentConfigAddition_ItemIEs_t *)calloc(1, sizeof(E2nodeComponentConfigAddition_ItemIEs_t));
e2configAdditionItem->id = ProtocolIE_ID_id_E2nodeComponentConfigAddition_Item;
e2configAdditionItem->criticality = Criticality_reject;
e2configAdditionItem->value.present = E2nodeComponentConfigAddition_ItemIEs__value_PR_E2nodeComponentConfigAddition_Item;

e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentInterfaceType = E2nodeComponentInterfaceType_ng;
e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentID.present = E2nodeComponentID_PR_e2nodeComponentInterfaceTypeNG;

auto *intfNG = (E2nodeComponentInterfaceNG_t *) calloc(1, sizeof(E2nodeComponentInterfaceNG_t));
  
OCTET_STRING_t nginterf;
nginterf.buf = (uint8_t*)calloc(1,8);
memcpy(nginterf.buf, (uint8_t *)"nginterf", 8);

nginterf.size = 8;
intfNG->amf_name = (AMFName_t)(nginterf);

e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentID.choice.e2nodeComponentInterfaceTypeNG = intfNG;

OCTET_STRING_t reqPart;
reqPart.buf = (uint8_t*)calloc(1,7);
memcpy(reqPart.buf, (uint8_t *)"reqpart", 7);
reqPart.size = 7;

e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentConfiguration.e2nodeComponentRequestPart = reqPart;

OCTET_STRING_t resPart;
resPart.buf = (uint8_t*)calloc(1,7);
memcpy(resPart.buf, (uint8_t *)"respart", 7);
resPart.size = 7;
e2configAdditionItem->value.choice.E2nodeComponentConfigAddition_Item.e2nodeComponentConfiguration.e2nodeComponentResponsePart = resPart;

ASN_SEQUENCE_ADD(&e2configIE->value.choice.E2nodeComponentConfigAddition_List, e2configAdditionItem);

  InitiatingMessage *inititingMsg = (InitiatingMessage *) calloc(1, sizeof(InitiatingMessage));
  inititingMsg->procedureCode = ProcedureCode_id_E2nodeConfigurationUpdate;
  inititingMsg->criticality = Criticality_reject;
  inititingMsg->value.present = InitiatingMessage__value_PR_E2nodeConfigurationUpdate;
  ASN_SEQUENCE_ADD(&inititingMsg->value.choice.E2nodeConfigurationUpdate.protocolIEs.list, e2txidIE);
  ASN_SEQUENCE_ADD(&inititingMsg->value.choice.E2nodeConfigurationUpdate.protocolIEs.list, e2configIE);
  e2ap_pdu->present = E2AP_PDU_PR_initiatingMessage;
  e2ap_pdu->choice.initiatingMessage = inititingMsg;
}

void encoding::generate_e2apv1_setup_response(E2AP_PDU_t *e2ap_pdu) {

  E2setupResponseIEs *resp_ies1 = (E2setupResponseIEs_t*)calloc(1, sizeof(E2setupResponseIEs_t));
  E2setupResponseIEs *resp_ies2 = (E2setupResponseIEs_t*)calloc(1, sizeof(E2setupResponseIEs_t));
  E2setupResponseIEs *resp_ies3 = (E2setupResponseIEs_t*)calloc(1, sizeof(E2setupResponseIEs_t));

  uint8_t *buf = (uint8_t *)"gnb1";

  BIT_STRING_t *ricid_bstring = (BIT_STRING_t*)calloc(1,sizeof(BIT_STRING_t));
  ricid_bstring->buf = buf;
  ricid_bstring->size = 4;
  ricid_bstring->bits_unused = 0;

  uint8_t *buf2 = (uint8_t *)"plmn3";
  OCTET_STRING_t *plmn = (OCTET_STRING_t*)calloc(1,sizeof(OCTET_STRING_t));
  plmn->buf = buf2;
  plmn->size = 5;

  GlobalRIC_ID_t *globalricid = (GlobalRIC_ID_t*)calloc(1,sizeof(GlobalRIC_ID_t));
  globalricid->pLMN_Identity = *plmn;
  globalricid->ric_ID = *ricid_bstring;

  E2setupResponseIEs__value_PR pres1;
  pres1 = E2setupResponseIEs__value_PR_GlobalRIC_ID;
  
  resp_ies1->id = ProtocolIE_ID_id_GlobalRIC_ID;
  resp_ies1->criticality = 0;
  resp_ies1->value.present = pres1;
  resp_ies1->value.choice.GlobalRIC_ID = *globalricid;

  E2setupResponse_t *e2setupresp = (E2setupResponse_t*)calloc(1,sizeof(E2setupResponse_t));
  int ret = ASN_SEQUENCE_ADD(&e2setupresp->protocolIEs.list, resp_ies1);


  SuccessfulOutcome__value_PR pres;
  pres = SuccessfulOutcome__value_PR_E2setupResponse;
  SuccessfulOutcome_t *successoutcome = (SuccessfulOutcome_t*)calloc(1, sizeof(SuccessfulOutcome_t));
  successoutcome->procedureCode = 1;
  successoutcome->criticality = 0;
  successoutcome->value.present = pres;
  successoutcome->value.choice.E2setupResponse = *e2setupresp;

  E2AP_PDU_PR pres5 = E2AP_PDU_PR_successfulOutcome;
  
  e2ap_pdu->present = pres5;
  e2ap_pdu->choice.successfulOutcome = successoutcome;
  
}


void encoding::generate_e2apv1_subscription_request(E2AP_PDU *e2ap_pdu) {

  fprintf(stderr, "in sub 1\n");
  RICsubscriptionRequest_IEs_t *ricreqid = (RICsubscriptionRequest_IEs_t*)calloc(1, sizeof(RICsubscriptionRequest_IEs_t));
  fprintf(stderr, "in sub 2\n");  
  ASN_STRUCT_RESET(asn_DEF_RICsubscriptionRequest_IEs, ricreqid);
  fprintf(stderr, "in sub 3\n");  
  auto *ricsubrid = (RICsubscriptionRequest_IEs_t*)calloc(1, sizeof(RICsubscriptionRequest_IEs_t));
  fprintf(stderr, "in sub 4\n");  
  ASN_STRUCT_RESET(asn_DEF_RICsubscriptionRequest_IEs, ricsubrid);
  
  fprintf(stderr, "in generate sub\n");
  uint8_t *buf2 = (uint8_t *)"SubscriptionTriggers";
  fprintf(stderr, "in gen sub 1\n");
  OCTET_STRING_t *triggerdef = (OCTET_STRING_t*)calloc(1, sizeof(OCTET_STRING_t));
  triggerdef->buf = (uint8_t *)calloc(1,20);
  triggerdef->size = 20;  
  memcpy(triggerdef->buf, buf2, triggerdef->size);


  fprintf(stderr, "sub1\n");
  ProtocolIE_ID_t proto_id= ProtocolIE_ID_id_RICaction_ToBeSetup_Item;

  RICaction_ToBeSetup_ItemIEs__value_PR pres6;
  pres6 = RICaction_ToBeSetup_ItemIEs__value_PR_RICaction_ToBeSetup_Item;

  printf("sub2\n");

  uint8_t *buf5 = (uint8_t *)"ActionDef";

  OCTET_STRING_t *actdef = (OCTET_STRING_t*)calloc(1, sizeof(OCTET_STRING_t));
  actdef->buf = (uint8_t *)calloc(1,9);
  actdef->size = 9;  
  memcpy(triggerdef->buf, buf5, 9);

  auto *sa = (RICsubsequentAction_t *) calloc(1, sizeof(RICsubsequentAction_t));
  ASN_STRUCT_RESET(asn_DEF_RICsubsequentAction, sa);
  
  sa->ricTimeToWait = RICtimeToWait_w500ms;
  sa->ricSubsequentActionType = RICsubsequentActionType_continue;
    
  printf("sub3\n");

  RICaction_ToBeSetup_ItemIEs_t *action_item_ies = (RICaction_ToBeSetup_ItemIEs_t *)calloc(1, sizeof(RICaction_ToBeSetup_Item_t));
  action_item_ies->id = proto_id;
  action_item_ies->criticality = 0;

  action_item_ies->value.present = pres6;
  action_item_ies->value.choice.RICaction_ToBeSetup_Item.ricActionID = 5;
  action_item_ies->value.choice.RICaction_ToBeSetup_Item.ricActionType = RICactionType_report;
  action_item_ies->value.choice.RICaction_ToBeSetup_Item.ricActionDefinition = actdef;
  action_item_ies->value.choice.RICaction_ToBeSetup_Item.ricSubsequentAction = sa;


  printf("sub5\n");
  

  RICsubscriptionRequest_IEs__value_PR pres3;
  printf("sub6.1\n");
  pres3 = RICsubscriptionRequest_IEs__value_PR_RICsubscriptionDetails;
  ricsubrid->id = ProtocolIE_ID_id_RICsubscriptionDetails;
  printf("sub6.2\n");
  
  ricsubrid->criticality = 0;
  ricsubrid->value.present = pres3;
  printf("sub6.3\n");

  ricsubrid->value.choice.RICsubscriptionDetails.ricEventTriggerDefinition = *triggerdef;
  printf("sub6.4\n");
  
  ASN_SEQUENCE_ADD(&ricsubrid->value.choice.RICsubscriptionDetails.ricAction_ToBeSetup_List.list, action_item_ies);

  printf("sub7\n");


  ricreqid->id = ProtocolIE_ID_id_RICrequestID;
  ricreqid->criticality = 0;
  ricreqid->value.present = RICsubscriptionRequest_IEs__value_PR_RICrequestID;
  ricreqid->value.choice.RICrequestID.ricRequestorID = 22;
  ricreqid->value.choice.RICrequestID.ricInstanceID = 6;

  RICsubscriptionRequest_t *ricsubreq = (RICsubscriptionRequest_t*)calloc(1, sizeof(RICsubscriptionRequest_t));

  ASN_SEQUENCE_ADD(&ricsubreq->protocolIEs.list,ricreqid);
  ASN_SEQUENCE_ADD(&ricsubreq->protocolIEs.list,ricsubrid);



  InitiatingMessage__value_PR pres4;
  pres4 = InitiatingMessage__value_PR_RICsubscriptionRequest;
  InitiatingMessage_t *initmsg = (InitiatingMessage_t*)calloc(1, sizeof(InitiatingMessage_t));
  initmsg->procedureCode = ProcedureCode_id_RICsubscription;
  initmsg->criticality = Criticality_reject;
  initmsg->value.present = pres4;
  initmsg->value.choice.RICsubscriptionRequest = *ricsubreq;

  E2AP_PDU_PR pres5;
  pres5 = E2AP_PDU_PR_initiatingMessage;
  

  e2ap_pdu->present = pres5;
  e2ap_pdu->choice.initiatingMessage = initmsg;

  char error_buf[300] = {0, };
  size_t errlen = 0;
									  
  asn_check_constraints(&asn_DEF_E2AP_PDU, e2ap_pdu, error_buf, &errlen);
  printf("error length %d\n", errlen);
  printf("error buf %s\n", error_buf);


}

void encoding::generate_e2apv1_subscription_response_success(E2AP_PDU *e2ap_pdu, long reqActionIdsAccepted[],
						   long reqActionIdsRejected[], int accept_size, int reject_size,
						   long reqRequestorId, long reqInstanceId) {

  RICsubscriptionResponse_IEs_t *respricreqid =
    (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));
  
  respricreqid->id = ProtocolIE_ID_id_RICrequestID;
  respricreqid->criticality = 0;
  respricreqid->value.present = RICsubscriptionResponse_IEs__value_PR_RICrequestID;
  respricreqid->value.choice.RICrequestID.ricRequestorID = reqRequestorId;
  
  respricreqid->value.choice.RICrequestID.ricInstanceID = reqInstanceId;

  RICsubscriptionResponse_IEs_t *respfuncid =
    (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));
  respfuncid->id = ProtocolIE_ID_id_RANfunctionID;
  respfuncid->criticality = 0;
  respfuncid->value.present = RICsubscriptionResponse_IEs__value_PR_RANfunctionID;
  respfuncid->value.choice.RANfunctionID = (long)0;
  

  RICsubscriptionResponse_IEs_t *ricactionadmitted =
    (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));
  ricactionadmitted->id = ProtocolIE_ID_id_RICactions_Admitted;
  ricactionadmitted->criticality = 0;
  ricactionadmitted->value.present = RICsubscriptionResponse_IEs__value_PR_RICaction_Admitted_List;

  RICaction_Admitted_List_t* admlist = 
    (RICaction_Admitted_List_t*)calloc(1,sizeof(RICaction_Admitted_List_t));
  ricactionadmitted->value.choice.RICaction_Admitted_List = *admlist;
  if (admlist) free(admlist);


  int numAccept = accept_size;
  int numReject = reject_size;


  
  for (int i=0; i < numAccept ; i++) {
    fprintf(stderr, "in for loop i = %d\n", i);

    long aid = reqActionIdsAccepted[i];

    RICaction_Admitted_ItemIEs_t *admitie = (RICaction_Admitted_ItemIEs_t*)calloc(1,sizeof(RICaction_Admitted_ItemIEs_t));
    admitie->id = ProtocolIE_ID_id_RICaction_Admitted_Item;
    admitie->criticality = 0;
    admitie->value.present = RICaction_Admitted_ItemIEs__value_PR_RICaction_Admitted_Item;
    admitie->value.choice.RICaction_Admitted_Item.ricActionID = aid;
    
    ASN_SEQUENCE_ADD(&ricactionadmitted->value.choice.RICaction_Admitted_List.list, admitie);

  }

  RICsubscriptionResponse_t *ricsubresp = (RICsubscriptionResponse_t*)calloc(1,sizeof(RICsubscriptionResponse_t));
  ASN_SEQUENCE_ADD(&ricsubresp->protocolIEs.list, respricreqid);
  ASN_SEQUENCE_ADD(&ricsubresp->protocolIEs.list, respfuncid);
  ASN_SEQUENCE_ADD(&ricsubresp->protocolIEs.list, ricactionadmitted);
  

  if (numReject > 0) {

    RICsubscriptionResponse_IEs_t *ricactionrejected =
      (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));
    ricactionrejected->id = ProtocolIE_ID_id_RICactions_NotAdmitted;
    ricactionrejected->criticality = 0;
    ricactionrejected->value.present = RICsubscriptionResponse_IEs__value_PR_RICaction_NotAdmitted_List;
    
    RICaction_NotAdmitted_List_t* rejectlist = 
      (RICaction_NotAdmitted_List_t*)calloc(1,sizeof(RICaction_NotAdmitted_List_t));
    ricactionadmitted->value.choice.RICaction_NotAdmitted_List = *rejectlist;
    
    for (int i=0; i < numReject; i++) {
      fprintf(stderr, "in for loop i = %d\n", i);
      
      long aid = reqActionIdsRejected[i];
      
      RICaction_NotAdmitted_ItemIEs_t *noadmitie = (RICaction_NotAdmitted_ItemIEs_t*)calloc(1,sizeof(RICaction_NotAdmitted_ItemIEs_t));
      noadmitie->id = ProtocolIE_ID_id_RICaction_NotAdmitted_Item;
      noadmitie->criticality = 0;
      noadmitie->value.present = RICaction_NotAdmitted_ItemIEs__value_PR_RICaction_NotAdmitted_Item;
      noadmitie->value.choice.RICaction_NotAdmitted_Item.ricActionID = aid;
      
      ASN_SEQUENCE_ADD(&ricactionrejected->value.choice.RICaction_NotAdmitted_List.list, noadmitie);
      ASN_SEQUENCE_ADD(&ricsubresp->protocolIEs.list, ricactionrejected);      
    }
  }


  SuccessfulOutcome__value_PR pres2;
  pres2 = SuccessfulOutcome__value_PR_RICsubscriptionResponse;
  SuccessfulOutcome_t *successoutcome = (SuccessfulOutcome_t*)calloc(1, sizeof(SuccessfulOutcome_t));
  successoutcome->procedureCode = ProcedureCode_id_RICsubscription;
  successoutcome->criticality = 0;
  successoutcome->value.present = pres2;
  successoutcome->value.choice.RICsubscriptionResponse = *ricsubresp;
  if (ricsubresp) free(ricsubresp);

  E2AP_PDU_PR pres5 = E2AP_PDU_PR_successfulOutcome;
  
  e2ap_pdu->present = pres5;
  e2ap_pdu->choice.successfulOutcome = successoutcome;

  char error_buf[300] = {0, };
  size_t errlen = 0;

  asn_check_constraints(&asn_DEF_E2AP_PDU, e2ap_pdu, error_buf, &errlen);
  printf("error length %d\n", errlen);
  printf("error buf %s\n", error_buf);

  printf("now printing xer of subscription response\n");
  xer_fprint(stderr, &asn_DEF_E2AP_PDU, e2ap_pdu);
  printf("done printing xer of subscription response\n");  

  
}

void encoding::generate_e2apv1_subscription_response(E2AP_PDU *e2ap_pdu, E2AP_PDU *sub_req_pdu) {

  //Gather details of the request

  RICsubscriptionRequest_t orig_req =
    sub_req_pdu->choice.initiatingMessage->value.choice.RICsubscriptionRequest;
  
  RICsubscriptionResponse_IEs_t *ricreqid =
    (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));
					   
  int count = orig_req.protocolIEs.list.count;
  int size = orig_req.protocolIEs.list.size;
  
  RICsubscriptionRequest_IEs_t **ies = (RICsubscriptionRequest_IEs_t**)orig_req.protocolIEs.list.array;

  fprintf(stderr, "count%d\n", count);
  fprintf(stderr, "size%d\n", size);

  RICsubscriptionRequest_IEs__value_PR pres;

  long responseRequestorId;
  long responseInstanceId;
  long responseActionId;

  std::vector<long> actionIds;

  for (int i=0; i < count; i++) {
    RICsubscriptionRequest_IEs_t *next_ie = ies[i];
    pres = next_ie->value.present;
    
    fprintf(stderr, "next present value %d\n", pres);

    switch(pres) {
    case RICsubscriptionRequest_IEs__value_PR_RICrequestID:
      {
	RICrequestID_t reqId = next_ie->value.choice.RICrequestID;
	long requestorId = reqId.ricRequestorID;
	long instanceId = reqId.ricInstanceID;
	fprintf(stderr, "requestorId %d\n", requestorId);
	fprintf(stderr, "instanceId %d\n", instanceId);
	responseRequestorId = requestorId;
	responseInstanceId = instanceId;
		
	break;
      }
    case RICsubscriptionRequest_IEs__value_PR_RANfunctionID:
      break;
    case RICsubscriptionRequest_IEs__value_PR_RICsubscriptionDetails:
      {
	RICsubscriptionDetails_t subDetails = next_ie->value.choice.RICsubscriptionDetails; 
	RICeventTriggerDefinition_t triggerDef = subDetails.ricEventTriggerDefinition;
	RICactions_ToBeSetup_List_t actionList = subDetails.ricAction_ToBeSetup_List;
	
	int actionCount = actionList.list.count;
	fprintf(stderr, "action count%d\n", actionCount);

	auto **item_array = actionList.list.array;

	for (int i=0; i < actionCount; i++) {
	  //RICaction_ToBeSetup_Item_t
	  auto *next_item = item_array[i];
	  RICactionID_t actionId = ((RICaction_ToBeSetup_ItemIEs*)next_item)->value.choice.RICaction_ToBeSetup_Item.ricActionID;
	  fprintf(stderr, "Next Action ID %ld\n", actionId);
	  responseActionId = actionId;
	  actionIds.push_back(responseActionId);
	}
	
	break;
      }
    }
    
  }

  fprintf(stderr, "After Processing Subscription Request\n");

  fprintf(stderr, "requestorId %d\n", responseRequestorId);
  fprintf(stderr, "instanceId %d\n", responseInstanceId);


  for (int i=0; i < actionIds.size(); i++) {
    fprintf(stderr, "Action ID %d %ld\n", i, actionIds.at(i));
    
  }


  RICsubscriptionResponse_IEs_t *respricreqid =
    (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));
  
  respricreqid->id = ProtocolIE_ID_id_RICrequestID;
  respricreqid->criticality = 0;
  respricreqid->value.present = RICsubscriptionResponse_IEs__value_PR_RICrequestID;
  respricreqid->value.choice.RICrequestID.ricRequestorID = responseRequestorId;
  
  respricreqid->value.choice.RICrequestID.ricInstanceID = responseInstanceId;


  RICsubscriptionResponse_IEs_t *ricactionadmitted =
    (RICsubscriptionResponse_IEs_t*)calloc(1, sizeof(RICsubscriptionResponse_IEs_t));
  ricactionadmitted->id = ProtocolIE_ID_id_RICactions_Admitted;
  ricactionadmitted->criticality = 0;
  ricactionadmitted->value.present = RICsubscriptionResponse_IEs__value_PR_RICaction_Admitted_List;

  RICaction_Admitted_List_t* admlist = 
    (RICaction_Admitted_List_t*)calloc(1,sizeof(RICaction_Admitted_List_t));
  ricactionadmitted->value.choice.RICaction_Admitted_List = *admlist;

  for (int i=0; i < actionIds.size(); i++) {
    fprintf(stderr, "in for loop i = %d\n", i);

    long aid = actionIds.at(i);

    RICaction_Admitted_ItemIEs_t *admitie = (RICaction_Admitted_ItemIEs_t*)calloc(1,sizeof(RICaction_Admitted_ItemIEs_t));
    admitie->id = ProtocolIE_ID_id_RICaction_Admitted_Item;
    admitie->criticality = 0;
    admitie->value.present = RICaction_Admitted_ItemIEs__value_PR_RICaction_Admitted_Item;
    admitie->value.choice.RICaction_Admitted_Item.ricActionID = aid;
    
    ASN_SEQUENCE_ADD(&ricactionadmitted->value.choice.RICaction_Admitted_List.list, admitie);

  }


  RICsubscriptionResponse_t *ricsubresp = (RICsubscriptionResponse_t*)calloc(1,sizeof(RICsubscriptionResponse_t));
  
  ASN_SEQUENCE_ADD(&ricsubresp->protocolIEs.list, respricreqid);
  ASN_SEQUENCE_ADD(&ricsubresp->protocolIEs.list, ricactionadmitted);


  SuccessfulOutcome__value_PR pres2;
  pres2 = SuccessfulOutcome__value_PR_RICsubscriptionResponse;
  SuccessfulOutcome_t *successoutcome = (SuccessfulOutcome_t*)calloc(1, sizeof(SuccessfulOutcome_t));
  successoutcome->procedureCode = ProcedureCode_id_RICsubscription;
  successoutcome->criticality = 0;
  successoutcome->value.present = pres2;
  successoutcome->value.choice.RICsubscriptionResponse = *ricsubresp;

  E2AP_PDU_PR pres5 = E2AP_PDU_PR_successfulOutcome;
  
  e2ap_pdu->present = pres5;
  e2ap_pdu->choice.successfulOutcome = successoutcome;

  char error_buf[300] = {0, };
  size_t errlen = 0;

  asn_check_constraints(&asn_DEF_E2AP_PDU, e2ap_pdu, error_buf, &errlen);
  printf("error length %d\n", errlen);
  printf("error buf %s\n", error_buf);


  
}

void encoding::generate_e2apv1_indication_request_parameterized(E2AP_PDU *e2ap_pdu,
								long requestorId,
								long instanceId,
								long ranFunctionId,
								long actionId,
								long seqNum,
								uint8_t *ind_header_buf,
								int header_length,
								uint8_t *ind_message_buf,
								int message_length) {
  
  fprintf(stderr, "ind1\n");
  RICindication_IEs_t *ricind_ies = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
  RICindication_IEs_t *ricind_ies2 = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
  RICindication_IEs_t *ricind_ies3 = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
  RICindication_IEs_t *ricind_ies4 = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
  RICindication_IEs_t *ricind_ies5 = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
  RICindication_IEs_t *ricind_ies6 = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
  RICindication_IEs_t *ricind_ies7 = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));
  RICindication_IEs_t *ricind_ies8 = (RICindication_IEs_t*)calloc(1, sizeof(RICindication_IEs_t));

  RICindication_IEs__value_PR pres3;

  pres3 = RICindication_IEs__value_PR_RICrequestID;
  ricind_ies->id = ProtocolIE_ID_id_RICrequestID;
  ricind_ies->criticality = 0;
  ricind_ies->value.present = pres3;
  ricind_ies->value.choice.RICrequestID.ricRequestorID = requestorId;
  ricind_ies->value.choice.RICrequestID.ricInstanceID = instanceId;

  fprintf(stderr, "ind2\n");

  pres3 = RICindication_IEs__value_PR_RANfunctionID;
  ricind_ies2->id = ProtocolIE_ID_id_RANfunctionID;
  ricind_ies2->criticality = 0;
  ricind_ies2->value.present = pres3;
  ricind_ies2->value.choice.RANfunctionID = ranFunctionId;

  
  ricind_ies3->id = ProtocolIE_ID_id_RICactionID;
  ricind_ies3->criticality = 0;
  pres3 =  RICindication_IEs__value_PR_RICactionID;
  ricind_ies3->value.present = pres3;
  ricind_ies3->value.choice.RICactionID = actionId;


  pres3 = RICindication_IEs__value_PR_RICindicationSN;
  ricind_ies4->id = ProtocolIE_ID_id_RICindicationSN;
  ricind_ies4->criticality = 0;
  ricind_ies4->value.present = pres3;
  ricind_ies4->value.choice.RICindicationSN = seqNum;

  //Indication type is REPORT
  pres3 = RICindication_IEs__value_PR_RICindicationType;
  ricind_ies5->id = ProtocolIE_ID_id_RICindicationType;
  ricind_ies5->criticality = 0;
  ricind_ies5->value.present = pres3;
  ricind_ies5->value.choice.RICindicationType = 0;

  fprintf(stderr, "ind3\n");

  ricind_ies6->value.choice.RICindicationHeader.buf = (uint8_t*)calloc(1,header_length);

  pres3 = RICindication_IEs__value_PR_RICindicationHeader;
  ricind_ies6->id = ProtocolIE_ID_id_RICindicationHeader;
  ricind_ies6->criticality = 0;
  ricind_ies6->value.present = pres3;
  ricind_ies6->value.choice.RICindicationHeader.size = header_length;
  memcpy(ricind_ies6->value.choice.RICindicationHeader.buf, ind_header_buf, header_length);
  
  ricind_ies7->value.choice.RICindicationMessage.buf = (uint8_t*)calloc(1,8192);


  

  pres3 = RICindication_IEs__value_PR_RICindicationMessage;
  ricind_ies7->id = ProtocolIE_ID_id_RICindicationMessage;
  fprintf(stderr, "after encoding message 1\n");

  ricind_ies7->criticality = 0;
  ricind_ies7->value.present = pres3;

  fprintf(stderr, "after encoding message 2\n");

  fprintf(stderr, "after encoding message 3\n");      
  ricind_ies7->value.choice.RICindicationMessage.size = message_length;

  fprintf(stderr, "after encoding message 4\n");
  memcpy(ricind_ies7->value.choice.RICindicationMessage.buf, ind_message_buf, message_length);

  fprintf(stderr, "after encoding message 5\n");

  uint8_t *cpid_buf = (uint8_t *)"cpid";
  OCTET_STRING_t cpid_str;

  printf("5.1\n");

  int cpid_buf_len = strlen((char*)cpid_buf);
  pres3 = RICindication_IEs__value_PR_RICcallProcessID;
  ricind_ies8->id = ProtocolIE_ID_id_RICcallProcessID;

  ricind_ies8->criticality = 0;
  ricind_ies8->value.present = pres3;

  ricind_ies8->value.choice.RICcallProcessID.buf = (uint8_t*)calloc(1,cpid_buf_len);
  ricind_ies8->value.choice.RICcallProcessID.size = cpid_buf_len;

  memcpy(ricind_ies8->value.choice.RICcallProcessID.buf, cpid_buf, cpid_buf_len);

  printf("5.2\n");

  RICindication_t *ricindication = (RICindication_t*)calloc(1, sizeof(RICindication_t));

  
  int ret;

  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies);
  
  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies2);

  printf("5.3\n");

  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies3);

  printf("5.35\n");
  
  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies4);

  printf("5.36\n");
  
  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies5);

  printf("5.4\n");
  
  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies6);

  printf("5.5\n");

  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies7);  
  
  ret = ASN_SEQUENCE_ADD(&ricindication->protocolIEs.list, ricind_ies8);    


  InitiatingMessage__value_PR pres4;
  pres4 = InitiatingMessage__value_PR_RICindication;
  InitiatingMessage_t *initmsg = (InitiatingMessage_t*)calloc(1, sizeof(InitiatingMessage_t));
  initmsg->procedureCode = 5;
  initmsg->criticality = 1;
  initmsg->value.present = pres4;
  initmsg->value.choice.RICindication = *ricindication;
  if (ricindication) free(ricindication);

  E2AP_PDU_PR pres5;
  pres5 = E2AP_PDU_PR_initiatingMessage;
  
  e2ap_pdu->present = pres5;
  e2ap_pdu->choice.initiatingMessage = initmsg;
  char error_buf[300] = {0, };
  size_t errlen = 0;

  asn_check_constraints(&asn_DEF_E2AP_PDU, e2ap_pdu, error_buf, &errlen);
  printf("error length %d\n", errlen);
  printf("error buf %s\n", error_buf);

  xer_fprint(stderr, &asn_DEF_E2AP_PDU, e2ap_pdu);  

}


