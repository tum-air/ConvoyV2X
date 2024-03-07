//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __CONVOY_ARCHITECTURE_CONVOYCONTROL_H_
#define __CONVOY_ARCHITECTURE_CONVOYCONTROL_H_

#include <omnetpp.h>
#include "common/defs.h"
#include "messages/ConvoyControlService_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class ConvoyControl : public omnetpp::cSimpleModule, public omnetpp::cListener
{
  public:
    ~ConvoyControl();
    ConvoyControl();
    enum AgentStatus {STATUS_INIT_SCANNING, STATUS_INIT_LOCALIZING, STATUS_INIT_RUN};
    virtual void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, omnetpp::cObject *) override;
    void getPublisherNodeID(std::set<int> &publisher_node_id);
    void getPublisherNodeX(std::map<int, double> &publisher_node_x);
    void getPublisherNodeY(std::map<int, double> &publisher_node_y);
    void getPublisherNodeFOV(std::map<int, double> &publisher_node_fov);
    void getPublisherNodeDirection(std::map<int, int> &publisher_node_direction);
    AgentStatus getAgentStatus();
    Role getClusterRole();
    unsigned short getManagerId();
    unsigned short getMemberId();
    unsigned short getGatewayId();
    double getTxp();
    double getTxpGw();
    ConvoyDirection getConvoyDirection();
    int getConvoyIdCCS();
    int getClusterIdCCS();

  private:
    ConvoyDirection _convoy_direction;
    double _cluster_broadcast_scan_duration;
    double _cluster_broadcast_scan_interval;
    double _cluster_init_localization_interval;
    double _publisher_list_update_interval;
    double _agent_run_mode_update_interval;
    double _ccs_broadcast_interval;
    omnetpp::cMessage* _cluster_broadcast_scan_trigger;
    omnetpp::cMessage* _cluster_broadcast_scan_timeout;
    omnetpp::cMessage* _localization_complete_trigger;
    omnetpp::cMessage* _run_mode_update_trigger;
    omnetpp::cMessage* _publisher_list_update_trigger;
    AgentStatus _agent_status;
    bool _is_cluster_manager;
    omnetpp::cModule* _manager_module;
    omnetpp::cModule* _member_module;
    omnetpp::cModule* _gateway_module;
    unsigned short _manager_id;
    unsigned short _member_id;
    unsigned short _gateway_id;
    StationType _station_type;
    Role _cluster_role;

    int _max_n_clusters;

    int _convoy_id;
    bool _convoy_full;
    bool _cluster_full;
    Role _convoy_role;
    int _convoy_id_gw;

    int _cluster_id;
    int _cluster_id_gw;

    std::set<int> _broadcast_rx_cluster_id;
    std::map<int, int> _broadcast_rx_convoy_id;
    std::map<int, bool> _broadcast_rx_cluster_full;
    std::map<int, bool> _broadcast_rx_convoy_full;
    std::map<int, int> _broadcast_rx_cluster_direction;
    std::map<int, double> _broadcast_rx_cluster_rssi;

    std::set<int> _broadcast_rx_publisher_id;
    std::map<int, double> _broadcast_rx_publisher_x;
    std::map<int, double> _broadcast_rx_publisher_y;
    std::map<int, double> _broadcast_rx_publisher_fov;
    std::map<int, int> _broadcast_rx_publisher_direction;

    double _txp{0.0};
    double _txp_gw{0.0};

    int _convoy_id_ccs{0};
    int _cluster_id_ccs{0};

  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
/*
    ConvoyDirection getInitialNodeDirection();
    ConvoyDirection getCurrentConvoyDirection();
    void startInitialLocalization();
    bool gatewayCheck();
    bool newClusterBroadcastReceived();
*/
    void createNewClusterManager();
    void createNewClusterMember();
    void createNewClusterGateway();
    void setNewLocation(omnetpp::cModule* module, Role role);
    void initCluster();
    void initModeUpdates(omnetpp::cMessage *msg);
    void clearLocalBroadcastRecord();
    void setupInitialRole(bool broadcast_received);
    void setAsConvoyMember(int convoy_id);
    void setAsClusterManager(int cluster_id);
    void setAsConvoyManager(int convoy_id);
    void setAsClusterMember(unsigned short cluster_id, double cluster_rssi);
    void setAsConvoyGateway(int convoy_id);
    void connectToWrapperNode(omnetpp::cModule* cluster_module);
    void removeConnectionsToWrapperNode(omnetpp::cModule* cluster_module);
    void setClusterBroadcastParameters();
    void runModeUpdates();
    void receiveCCSMessage(ConvoyControlService *ccs_message);
    void enforceMembership(Role convoy_role, Role cluster_role, int convoy_id, int cluster_id, int convoy_id_gw, int cluster_id_gw);
    void enforceMaxResBlks(int max_res_blks);
    void enforceTxPower(Role cluster_role, double tx_power, double tx_power_gw);
    void sendCCSMessage();
    void removeExistingClusterManager();
    void attachClusterGateway(unsigned short cluster_id_gw, double cluster_rssi_gw);
    void changeConvoy(Role convoy_role, int convoy_id, int convoy_id_gw);
    void changeCluster(Role cluster_role, int convoy_id, int cluster_id, int convoy_id_gw, int cluster_id_gw);
    void changeClusterRole(Role old_cluster_role, Role cluster_role, int convoy_id, int cluster_id, int convoy_id_gw, int cluster_id_gw);
    void detachClusterGateway();
};

} // namespace convoy_architecture

#endif
