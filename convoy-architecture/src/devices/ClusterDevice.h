
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
    ClusterDeviceType _type {ClusterDeviceType::UNKNOWN_DEVICE};
    int _address {0};
    int _address_manager {0};

  public:
    ClusterDeviceType getType() {return _type;}
    int getAddress() {return _address;}
    int getAddressOfManager() {return _address_manager;}
    void disconnect();
    void remove();
    void configureAndStartAs(ClusterDeviceType device_type, DeviceInterfaceType interface_type);
};

} // namespace convoy_architecture

#endif
