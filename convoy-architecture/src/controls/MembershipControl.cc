
#include "MembershipControl.h"
#include "common/binder/Binder.h"

namespace convoy_architecture {

Define_Module(MembershipControl);

void MembershipControl::initialize()
{
    // TODO - Generated method body
}

void MembershipControl::handleMessage(omnetpp::cMessage *msg)
{
    // TODO - Generated method body
}

const std::vector<Publication>& MembershipControl::readPublishers() const
{
    return _publishers;
}

ClusterDeviceType MembershipControl::clusterMatch(int cluster) {
    // TODO
    return(ClusterDeviceType::UNKNOWN_DEVICE);
}

void MembershipControl::disconnectClusterDevice(ClusterDevice* cluster_device) {
    if(cluster_device != nullptr) {
        if(cluster_device->isModule()) {
            std::vector<std::string> gates = cluster_device->getGateNames();
            std::for_each(std::begin(gates), std::end(gates), [cluster_device] (std::string gate_name) {
                omnetpp::cGate* gate = cluster_device->gate(gate_name.c_str());
                if(gate->isConnected())
                    gate->disconnect();
            });
        }
    }
}

void MembershipControl::removeClusterDevice(ClusterDevice* cluster_device) {
    if(cluster_device != nullptr) {
        if(cluster_device->isModule())
            cluster_device->deleteModule();
    }
}

MembershipControl::~MembershipControl()
{
    /* TODO
    cancelAndDelete(_outstanding_scheduled_messages);
    */

    // Disconnect and remove associated cluster devices
    disconnectClusterDevice(_device_pr);
    disconnectClusterDevice(_device_gw);
    removeClusterDevice(_device_pr);
    removeClusterDevice(_device_gw);

    // Inform binder if the primary device is a master
    if(_device_type_pr == ClusterDeviceType::MASTER_DEVICE) {
        Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
        binder->deNotifyCluster(_id_gnb);
    }
}

} // namespace convoy_architecture
