
#include "ClusterDevice.h"
#include "common/binder/Binder.h"

namespace convoy_architecture {

Define_Module(ClusterDevice);

void ClusterDevice::initialize()
{
    // TODO - Generated method body
}

void ClusterDevice::handleMessage(omnetpp::cMessage *msg)
{
    // TODO - Generated method body
}

void ClusterDevice::disconnect() {
    if(isModule()) {
        std::vector<std::string> gates = getGateNames();
        std::for_each(std::begin(gates), std::end(gates), [this] (std::string gate_name) {
            if(this->gate(gate_name.c_str())->isConnected())
                this->gate(gate_name.c_str())->disconnect();
        });
    }
}

void ClusterDevice::remove() {
    if(isModule())
        deleteModule();

    // Inform binder if the primary device is a master
    if(_type == ClusterDeviceType::MASTER_DEVICE) {
        Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
        binder->deNotifyCluster(_address_manager);
    }
}

void ClusterDevice::configureAndStartAs(ClusterDeviceType device_type, DeviceInterfaceType interface_type) {
    // Finalize device configuration
    if(interface_type != DeviceInterfaceType::PRIMARY_INTERFACE) {
        par("isPrimary") = false;
    }
    if(device_type != ClusterDeviceType::MASTER_DEVICE) {
        par("isMaster") = false;
        par("nicType") = "NRNicUe";
        par("nodeType") = "UE";
    }
    finalizeParameters();
    buildInside();

    // Start device
    scheduleStart(omnetpp::simTime());
    callInitialize();

    // Use macNodeId assigned by binder as device address
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    _address = par("macNodeId").intValue();
    _address_manager = binder->getNextHop(_address);
}

} // namespace convoy_architecture
