# <PROJECT_NAME>: e2sim Integration with O-RAN RIC (Bare Metal + Minikube) on Ubuntu 22.04 

## 1. Overview

This document describes how to deploy and configure **e2sim** on a **bare-metal host** and integrate it with an ORAN nearRT RIC running inside a **Minikube (or Kubernetes) cluster**. It is intended as a generic, open-source guide that can be adapted to different lab environments.

### 1.1 Topology Assumptions

- NearRT RIC, nonRT RIC / SMO, xApps, and (optionally) packet core run in a Kubernetes cluster (e.g., Minikube).
- e2sim runs on a separate baremetal Linux server reachable over IP from the Kubernetes node(s).
- E2 interface uses SCTP on a configurable port (commonly 36421, but may differ).
- Firewalls and security policies allow SCTP traffic between the e2sim host and the RIC E2 endpoint.

### 1.2 Notation and Placeholders

Replace all anglebracket placeholders with values appropriate for your deployment:

- `<E2SIM_HOST_IP>`: IP address of the baremetal server running e2sim.  
- `<RIC_E2_IP>`: IP address (or external IP) of the RIC’s E2 endpoint (K8s node or LoadBalancer).  
- `<RIC_E2_PORT>`: SCTP port for E2 on the RIC side (NodePort/LB port).  
- `<PLMN_MCC>` / `<PLMN_MNC>`: PLMN ID (MCC/MNC) used in your RIC and RAN configuration.  
- `<GNB_ID>`: gNB ID or E2 node ID advertised by e2sim.  
- `<RAN_FUNCTION_ID_KPM>`, `<RAN_FUNCTION_ID_RC>`, etc.: RAN Function IDs supported by your e2sim build.  
- `<E2SIM_REPO_URL>`: Git URL of the e2sim source repository.  
- `<LINUX_DISTRO>`: Linux distribution (e.g., Ubuntu 22.04, CentOS 8).  


---

## 2. Prerequisites

### 2.1 NearRT RIC / Kubernetes Cluster

- Kubernetes cluster (e.g., Minikube) is installed and running.
- NearRT RIC components (E2Term, E2 Manager, platform services) are deployed and healthy.
- xApps relevant to use case are:
  - Onboarded to the RIC.
  - Deployed and running.

Recommended checks (examples):

- `kubectl get pods -n <RIC_NAMESPACE>`
- RIC GUI or CLI shows:
  - All core services in **Running** state.
  - xApps in **Running** or **Active** state.

### 2.2 BareMetal e2sim Host

- OS: `<LINUX_DISTRO>` on a physical or dedicated server.
- Network:
  - Static or stable IP `<E2SIM_HOST_IP>`.
  - Bidirectional IP reachability to `<RIC_E2_IP>`.
- Time synchronization:
  - NTP or equivalent (e.g., chrony, systemd-timesyncd) configured to avoid large clock drift.
- Administrative access:
  - SSH access and `sudo` privileges to install dependencies and build e2sim.

---

## 3. Minikube / RIC Configuration

### 3.1 Expose the E2 Endpoint

1. Identify the E2Term/E2 service in the RIC namespace:

   - `kubectl get svc -n <RIC_NAMESPACE>`

2. Ensure the service type is externally reachable:

   - Option A: `NodePort`  
   - Option B: `LoadBalancer` (if supported by environment)

3. Record:

   - `<RIC_E2_IP>`: Node IP or LoadBalancer IP.  
   - `<RIC_E2_PORT>`: Exposed SCTP port for E2.

### 3.2 RIC Expectations for E2 Nodes

Confirm or configure in the RIC:

- PLMN ID:
  - MCC: `<PLMN_MCC>`
  - MNC: `<PLMN_MNC>`
- Allowed / expected E2 node(s):
  - E2 node (gNB) ID: `<GNB_ID>`
  - Optional: humanreadable node name: `<E2_NODE_NAME>`
- RAN Functions expected:
  - Example:  
    - KPM RAN Function ID: `<RAN_FUNCTION_ID_KPM>`  
    - RC RAN Function ID: `<RAN_FUNCTION_ID_RC>`

If your RIC uses an allowlist or configuration file for E2 nodes, add the e2sim node accordingly.

---

## 4. Prepare the e2sim BareMetal Host

### 4.1 Install System Dependencies (Example: Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install -y \
  git build-essential cmake autoconf automake libtool pkg-config \
  libsctp-dev lksctp-tools
  
For other distributions, install the equivalent development tools and SCTP libraries.

4.2 Network and Time Sync Checks
Verify connectivity to the RIC E2 endpoint:
ping <RIC_E2_IP>

Ensure outbound SCTP is not blocked:

Validate host firewall (e.g., ufw, firewalld) rules.
Validate any network firewalls between <E2SIM_HOST_IP> and <RIC_E2_IP>.
Confirm time synchronization is active:

timedatectl status or distributionspecific commands.
5. Download and Build e2sim
5.1 Clone the Repository
mkdir -p ~/e2sim && cd ~/e2sim
git clone <E2SIM_REPO_URL> .

If a specific branch or tag is required:
git checkout <BRANCH_OR_TAG>

5.2 Build
Typical CMakebased build flow (adapt as needed to repo):
mkdir -p build && cd build
cmake ..
make -j"$(nproc)"

Expected result:

e2sim (or similarly named) binary produced in the build directory (e.g., ~/e2sim/build/e2sim).
Optional installation:
sudo cp e2sim /usr/local/bin/

6. Configure e2sim
The exact configuration method may be:

A configuration file (e.g., JSON, YAML, INI), or
Commandline arguments, or
A combination of both.
Adapt this section to match the e2sim implementation you are using.

6.1 Core Connection Parameters
Set the following to match your RIC exposure:

RIC E2 endpoint:
IP: <RIC_E2_IP>
Port: <RIC_E2_PORT>
Transport: SCTP
Local e2sim host:
Local IP: <E2SIM_HOST_IP> (if required explicitly).
Local port: <E2SIM_LOCAL_PORT> (if applicable; often autoassigned).
6.2 RAN Identity
Configure:

PLMN:
MCC: <PLMN_MCC>
MNC: <PLMN_MNC>
gNB / E2 Node:
ID: <GNB_ID>
Name: <E2_NODE_NAME>
Ensure the format (bit length, encoding) aligns with RIC expectations.

6.3 RAN Functions
Advertise RAN Functions consistent with the xApps you plan to run.

Examples:

KPM (Performance Measurements)
ID: <RAN_FUNCTION_ID_KPM>
Description / OID: <RAN_FUNCTION_OID_KPM>
RC (Control)
ID: <RAN_FUNCTION_ID_RC>
Description / OID: <RAN_FUNCTION_OID_RC>
Check that:

Function IDs match the RIC/xApp configuration.
Function descriptions and OIDs are compatible with the xApps’ expectations.
6.4 Logging and Runtime Options
Common parameters (adapt to actual implementation):

Log level: <LOG_LEVEL> (e.g., info, debug).
Number of cells: <NUM_CELLS>
Number of UEs (if simulated): <NUM_UES>
Measurement/indication interval: <INDICATION_INTERVAL_MS>
6.5 Security / TLS (If Enabled)
If E2 is secured in your deployment:

Client certificate: <E2SIM_CLIENT_CERT_PATH>
Client key: <E2SIM_CLIENT_KEY_PATH>
CA certificate / bundle: <E2_CA_CERT_PATH>
TLS parameters: <TLS_VERSION>, <CIPHER_SUITES> etc., as required.
Ensure RIC and e2sim agree on all security parameters.

7. Run e2sim and Verify E2 Setup
7.1 Start e2sim
cd ~/e2sim/build
./e2sim -c <E2SIM_CONFIG_FILE>

Example command (adapt to actual binary and config):

7.2 Validate Logs on e2sim Host
Look for:

Successful SCTP association to <RIC_E2_IP>:<RIC_E2_PORT>.
E2 SETUP REQUEST from e2sim.
E2 SETUP RESPONSE / SUCCESS from RIC.
On failure, note any error codes or protocol messages for troubleshooting.

7.3 Validate from RIC Perspective
Using RIC GUI or CLI:

Verify the new E2 node appears as CONNECTED/ACTIVE.
Confirm RAN Functions advertised by e2sim are visible for that node.
Check E2Term/E2Mgr logs for any warnings or errors.
8. xApp and SMO Integration
8.1 Confirm xApps Are Running
Verify xApps (e.g., KPM xApp, RC xApp) are:

Onboarded to the RIC.
Enabled and running.
Check using your RIC’s management UI or:
kubectl get pods -n <XAPP_NAMESPACE>

8.2 Create E2 Subscriptions
Using RIC GUI/CLI/SMO workflows (implementationdependent):

Select the e2sim E2 node (ID <GNB_ID>, name <E2_NODE_NAME>).
Select the desired RAN Function(s) (e.g., <RAN_FUNCTION_ID_KPM>, <RAN_FUNCTION_ID_RC>).
Configure subscription parameters:
Reporting period: <REPORTING_PERIOD_MS>
Measurement set or control target details as supported by the xApp.
Validate:

e2sim logs show subscription requests and acceptances.
xApp logs show active subscriptions and incoming indications or control messages.
8.3 NonRT RIC / SMO and Packet Core (Optional)
If your scenario includes SMO and packet core:

Ensure SMO can:
See the E2 node and its KPIs.
Apply policies that flow to nearRT RIC and then to e2sim.
Validate that UE traffic in the packet core (simulated or real) is reflected in measurements and KPIs produced by e2sim and consumed by xApps/SMO.
9. Troubleshooting Guide (Checklist)
9.1 Connectivity Issues
From e2sim host:
ping <RIC_E2_IP>
ss -nupa | grep sctp

Verify:

Host firewall rules allow SCTP from <E2SIM_HOST_IP> to <RIC_E2_IP>:<RIC_E2_PORT>.
Intermediate firewalls or security devices do not block SCTP.
9.2 E2 Setup Failures
Check RIC logs for E2Term/E2Mgr:
Mismatched PLMN (MCC/MNC).
Incorrect gNB ID or unsupported RAN Function.
Security/TLS issues (e.g., certificate validation failures).
Verify e2sim configuration:
Correct <RIC_E2_IP>, <RIC_E2_PORT>.
Proper PLMN and <GNB_ID> values.
9.3 xApp Subscription or Data Issues
Ensure:
Subscription targets the correct E2 node and RAN Function ID.
xApp decoders support the RAN Function OIDs and message formats emitted by e2sim.
Review:
xApp logs for subscription errors or decode failures.
e2sim logs for rejected or malformed subscription messages.
9.4 Performance and Stability
If experiencing frequent reconnects:
Check network stability between <E2SIM_HOST_IP> and <RIC_E2_IP>.
Validate SCTP tuning parameters if applicable.
If logs are too verbose:
Reduce <LOG_LEVEL> to info or warning.
10. Contribution Notes
This document is intended as a generic template for opensource use. When contributing to a project:

Replace placeholders with concrete examples or provide a sample configuration in an appendix.
Keep vendor or deploymentspecific details in separate sections or documents.


