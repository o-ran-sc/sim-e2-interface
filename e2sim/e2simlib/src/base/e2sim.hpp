#ifndef E2SIM_HPP
#define E2SIM_HPP

#include <unordered_map>

extern "C" {
#include "E2AP-PDU.h"
#include "OCTET_STRING.h"
}

typedef void (*SubscriptionCallback)(E2AP_PDU_t*);

class E2Sim;
class E2Sim {

private:

  std::unordered_map<long, OCTET_STRING_t*> ran_functions_registered;
  std::unordered_map<long, SubscriptionCallback> subscription_callbacks;

  void wait_for_sctp_data();
  
public:

  void generate_e2apv1_subscription_response_success(E2AP_PDU *e2ap_pdu, long reqActionIdsAccepted[], long reqActionIdsRejected[], int accept_size, int reject_size, long reqRequestorId, long reqInstanceId);

  void generate_e2apv1_indication_request_parameterized(E2AP_PDU *e2ap_pdu, long requestorId, long instanceId, long ranFunctionId, long actionId, long seqNum, uint8_t *ind_header_buf, int header_length, uint8_t *ind_message_buf, int message_length);  

  SubscriptionCallback get_subscription_callback(long func_id);
  
  void register_e2sm(long func_id, OCTET_STRING_t* ostr);

  void register_subscription_callback(long func_id, SubscriptionCallback cb);
  
  void encode_and_send_sctp_data(E2AP_PDU_t* pdu);

  int run_loop(int argc, char* argv[]);

};

#endif
