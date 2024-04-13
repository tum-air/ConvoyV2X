
#ifndef __CONVOY_ARCHITECTURE_MEMBERSHIPCONTROL_H_
#define __CONVOY_ARCHITECTURE_MEMBERSHIPCONTROL_H_

#include <omnetpp.h>
#include "common/defs.h"
#include "devices/ClusterDevice.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class MembershipControl : public omnetpp::cSimpleModule, public omnetpp::cListener
{
private:
    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;
    ClusterDevice* createInitClusterDevice(ClusterDeviceType device_type, DeviceInterfaceType interface_type);
    void setLocation(ClusterDevice* device, ClusterDeviceType type);

    MemberControlState _control_state {MemberControlState::INITIALIZING};
    std::vector<Publication> _publishers;

    //
    ClusterDevice* _device_pr {nullptr};
    ClusterDevice* _device_gw {nullptr};

public:
    bool isInitialized() {return (_control_state == MemberControlState::INITIALIZED);}
    const std::vector<Publication>& readPublishers() const;
    int getAddressOfManagerForPrimaryDevice() {return (_device_pr == nullptr)? 0 : _device_pr->getAddressOfManager();}
    int getAddressOfManagerForGatewayDevice() {return (_device_gw == nullptr)? 0 : _device_gw->getAddressOfManager();}
    bool addressMatchesPrimaryDevice(int address) {return (_device_pr == nullptr)? false : (address==_device_pr->getAddress());}
    bool addressMatchesGatewayDevice(int address) {return (_device_gw == nullptr)? false : (address==_device_gw->getAddress());}
    ClusterDeviceType clusterMatch(int cluster_address);
    void receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, omnetpp::cObject *) override;

    //
    ~MembershipControl();
};

} // namespace convoy_architecture

#endif
