
#ifndef __CONVOY_ARCHITECTURE_CLUSTERDEVICE_H_
#define __CONVOY_ARCHITECTURE_CLUSTERDEVICE_H_

#include <omnetpp.h>
#include "common/defs.h"

namespace convoy_architecture {

/**
 * Common cluster device
 */
class ClusterDevice : public omnetpp::cSimpleModule
{
  private:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    ClusterDeviceType _device_type {ClusterDeviceType::UNKNOWN_DEVICE};
    DeviceInterfaceType _interface_type {DeviceInterfaceType::UNKNOWN_INTERFACE};
    int _address {0};
    int _address_manager {0};
    int _address_convoy {0};

  public:
    ClusterDeviceType getDeviceType() {return _device_type;}
    int getAddress() {return _address;}
    int getAddressOfManager() {return _address_manager;}
    void disconnect();
    void remove();
    void setType(ClusterDeviceType device_type, DeviceInterfaceType interface_type);
    void configureAndStart(int direction, int address_convoy = 0);
    void updateBroadcastInfo(const Broadcast &broadcast);
    void updateBroadcastInfo(const std::vector<Publication> &publishers);
    void connectPrimaryDeviceTo(omnetpp::cModule *base_node);
    void connectGatewayDeviceTo(omnetpp::cModule *base_node);
    void joinNewCluster(int id_convoy, int id_cluster, double rssi);
};

} // namespace convoy_architecture

#endif
