
#include "MembershipControl.h"
#include "nodes/BaseNode.h"
#include "stack/phy/layer/NRPhyUe.h"

namespace convoy_architecture {

Define_Module(MembershipControl);

void MembershipControl::initialize()
{
    // register to get a notification when position changes
    getParentModule()->subscribe(inet::IMobility::mobilityStateChangedSignal, this);

    // Create primary cluster device
    // and initiate its role as a member for broadcast reception
    _device_pr = createInitClusterDevice(ClusterDeviceType::MEMBER_DEVICE, DeviceInterfaceType::PRIMARY_INTERFACE);

    // TODO
}

ClusterDevice* MembershipControl::createInitClusterDevice(ClusterDeviceType device_type, DeviceInterfaceType interface_type) {
    std::string device_name ="unknown";
    int device_index = 0;

    if(device_type == ClusterDeviceType::MEMBER_DEVICE) {
        device_name = "member";
    }

    // Create submodule vector first, if it doesn't already exist
    if(!getSystemModule()->hasSubmoduleVector(device_name.c_str()))
        getSystemModule()->addSubmoduleVector(device_name.c_str(), 1);
    else
    {
        device_index = getSystemModule()->getSubmoduleVectorSize(device_name.c_str());
        getSystemModule()->setSubmoduleVectorSize(device_name.c_str(), (device_index + 1));
    }

    // Create new cluster device module
    omnetpp::cModuleType *module_type = omnetpp::cModuleType::get("convoy_architecture.devices.ClusterDevice");
    ClusterDevice* device = check_and_cast<ClusterDevice*>(module_type->create(device_name.c_str(), getSystemModule(), device_index));

    // Initialize mobility
    setLocation(device, device_type);

    // Configure and start device
    device->configureAndStartAs(device_type, interface_type);

    // register to get a notification when broadcast messages arrive
    if(device_type != ClusterDeviceType::MASTER_DEVICE) {
        NRPhyUe *nic = check_and_cast<NRPhyUe *>(device->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        nic->subscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
    }

    return device;
}

void MembershipControl::setLocation(ClusterDevice* device, ClusterDeviceType type)
{
    double offset_x, offset_y;
    std::string color = "red";
    switch (type)
    {
    case ClusterDeviceType::MASTER_DEVICE:
        offset_x = 0.0;
        offset_y = -1.0;
        break;
    case ClusterDeviceType::GATEWAY_DEVICE:
        offset_x = -2.0;
        offset_y = -1.0;
        color = "lightblue";
        break;
    default:
        offset_x = 2.0;
        offset_y = -1.0;
        color = "yellow";
        break;
    }

    // Get current location of base node
    BaseNode* base_node = check_and_cast<BaseNode*>(getParentModule());
    inet::Coord location = base_node->getCurrentLocation();

    // Update display string
    omnetpp::cDisplayString display_string = device->getDisplayString();
    display_string.setTagArg("p", 0, (location.x + offset_x));
    display_string.setTagArg("p", 1, (location.y + offset_y));
    display_string.setTagArg("b", 3, color.c_str());
    display_string.setTagArg("b", 4, color.c_str());
    device->setDisplayString(display_string.str());
}

void MembershipControl::handleMessage(omnetpp::cMessage *msg)
{
    // TODO - Generated method body
}

const std::vector<Publication>& MembershipControl::readPublishers() const
{
    return _publishers;
}

ClusterDeviceType MembershipControl::clusterMatch(int cluster_address) {
    if(_device_pr != nullptr) {
        if(cluster_address == _device_pr->getAddressOfManager())
            return _device_pr->getType();
    }
    if(_device_gw != nullptr) {
        if(cluster_address == _device_gw->getAddressOfManager())
            return _device_gw->getType();
    }
    return(ClusterDeviceType::UNKNOWN_DEVICE);
}

void MembershipControl::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, omnetpp::cObject *)
{
    // TODO
}

MembershipControl::~MembershipControl()
{
    /* TODO
    cancelAndDelete(_outstanding_scheduled_messages);
    */

    // Disconnect and remove associated cluster devices
    if (_device_pr != nullptr) {
        _device_pr->disconnect();
        _device_pr->remove();
    }
    if (_device_gw != nullptr) {
        _device_gw->disconnect();
        _device_gw->remove();
    }
}

} // namespace convoy_architecture
