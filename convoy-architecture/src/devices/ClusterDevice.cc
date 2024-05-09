
#include "ClusterDevice.h"
#include "common/binder/Binder.h"
#include "stack/phy/layer/LtePhyEnbD2D.h"
#include "stack/phy/layer/NRPhyUe.h"

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
}

void ClusterDevice::setType(ClusterDeviceType device_type, DeviceInterfaceType interface_type){
    _device_type = device_type;
    _interface_type = interface_type;
}

void ClusterDevice::configureAndStart(int direction, int address_convoy) {
    _address_convoy = address_convoy;

    // Finalize device configuration
    par("convoyDirection") = direction;
    par("convoyID") = address_convoy;
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
    _address = (_device_type == ClusterDeviceType::MASTER_DEVICE)? par("macNodeId").intValue() : par("nrMacNodeId").intValue();
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

void ClusterDevice::connectPrimaryDeviceTo(omnetpp::cModule *base_node) {
    base_node->gate("outLlPublisher")->connectTo(gate("fromAppPublisher"));
    gate("toAppPublisher")->connectTo(base_node->gate("inLlPublisher"));

    base_node->gate("outLlSubscriber")->connectTo(gate("fromAppSubscriber"));
    gate("toAppSubscriber")->connectTo(base_node->gate("inLlSubscriber"));

    base_node->gate("outLlManeuver")->connectTo(gate("fromAppManeuver"));
    gate("toAppManeuver")->connectTo(base_node->gate("inLlManeuver"));

    base_node->gate("outLlMemberControl")->connectTo(gate("fromAppMemberControl"));
    gate("toAppMemberControl")->connectTo(base_node->gate("inLlMemberControl"));

    base_node->gate("outLlMemberReport")->connectTo(gate("fromAppMemberReport"));
    gate("toAppMemberReport")->connectTo(base_node->gate("inLlMemberReport"));
}

void ClusterDevice::connectGatewayDeviceTo(omnetpp::cModule *base_node) {
    base_node->gate("outLlPublisherGw")->connectTo(gate("fromAppPublisher"));
    gate("toAppPublisher")->connectTo(base_node->gate("inLlPublisherGw"));

    base_node->gate("outLlSubscriberGw")->connectTo(gate("fromAppSubscriber"));
    gate("toAppSubscriber")->connectTo(base_node->gate("inLlSubscriberGw"));

    base_node->gate("outLlManeuverGw")->connectTo(gate("fromAppManeuver"));
    gate("toAppManeuver")->connectTo(base_node->gate("inLlManeuverGw"));

    base_node->gate("outLlMemberControlGw")->connectTo(gate("fromAppMemberControl"));
    gate("toAppMemberControl")->connectTo(base_node->gate("inLlMemberControlGw"));

    base_node->gate("outLlMemberReportGw")->connectTo(gate("fromAppMemberReport"));
    gate("toAppMemberReport")->connectTo(base_node->gate("inLlMemberReportGw"));
}

void ClusterDevice::joinNewCluster(int id_convoy, int id_cluster, double rssi) {
    if(_device_type != ClusterDeviceType::MASTER_DEVICE) {
        NRPhyUe *nic = check_and_cast<NRPhyUe *>(getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        nic->attachToCluster(id_cluster, rssi);

        // Use macNodeId assigned by binder as device address
        _address = par("nrMacNodeId").intValue();
        _address_manager = id_cluster;
        _address_convoy = id_convoy;
    }
}

void ClusterDevice::detachFromCluster() {
    _address = 0;
    _address_manager = 0;
    _address_convoy = 0;
}

void ClusterDevice::setTxp(double txp) {
    if(_device_type == ClusterDeviceType::MASTER_DEVICE) {
        LtePhyEnbD2D *nic = check_and_cast<LtePhyEnbD2D *>(getSubmodule("cellularNic")->getSubmodule("phy"));
        nic->setTxPower(txp);
    }
    else {
        NRPhyUe *nic = check_and_cast<NRPhyUe *>(getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        nic->setTxPower(txp);
    }
}

double ClusterDevice::getTxp() {
    double txp;
    if(_device_type == ClusterDeviceType::MASTER_DEVICE) {
        LtePhyEnbD2D *nic = check_and_cast<LtePhyEnbD2D *>(getSubmodule("cellularNic")->getSubmodule("phy"));
        txp = nic->getTxPower();
    }
    else {
        NRPhyUe *nic = check_and_cast<NRPhyUe *>(getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        txp = nic->getTxPower();
    }

    return txp;
}

} // namespace convoy_architecture
