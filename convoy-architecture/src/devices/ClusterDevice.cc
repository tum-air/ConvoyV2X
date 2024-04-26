
#include "ClusterDevice.h"
#include "common/binder/Binder.h"
#include "stack/phy/layer/LtePhyEnbD2D.h"

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
    // Inform binder if the primary device is a master
    if(_device_type == ClusterDeviceType::MASTER_DEVICE) {
        Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
        binder->deRegisterCluster(_address_convoy, _address_manager);
    }

    if(isModule())
            deleteModule();

}

void ClusterDevice::setType(ClusterDeviceType device_type, DeviceInterfaceType interface_type){
    _device_type = device_type;
    _interface_type = interface_type;
}

void ClusterDevice::configureAndStart(int direction, int address_convoy) {
    _address_convoy = address_convoy;

    // Finalize device configuration
    par("convoyDirection") = direction;
    if(_interface_type != DeviceInterfaceType::PRIMARY_INTERFACE) {
        par("isPrimary") = false;
    }
    if(_device_type != ClusterDeviceType::MASTER_DEVICE) {
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

    // Update broadcast information
    if(_device_type == ClusterDeviceType::MASTER_DEVICE) {
        Broadcast broadcast {
            address_convoy,
            _address_manager,
            false,
            false,
            (ConvoyDirection) direction,
            0
        };
        std::vector<Publication> publishers {};
        updateBroadcastInfo(broadcast);
        updateBroadcastInfo(publishers);
    }
}

void ClusterDevice::updateBroadcastInfo(const Broadcast &broadcast)
{
    if(_device_type == ClusterDeviceType::MASTER_DEVICE) {
        LtePhyEnbD2D *nic = check_and_cast<LtePhyEnbD2D *>(getSubmodule("cellularNic")->getSubmodule("phy"));
        nic->updateBroadcastParameters(broadcast);
    }
}
void ClusterDevice::updateBroadcastInfo(const std::vector<Publication> &publishers) {
    if(_device_type == ClusterDeviceType::MASTER_DEVICE) {
        LtePhyEnbD2D *nic = check_and_cast<LtePhyEnbD2D *>(getSubmodule("cellularNic")->getSubmodule("phy"));
        nic->updateBroadcastParameters(publishers);
    }
}


} // namespace convoy_architecture
