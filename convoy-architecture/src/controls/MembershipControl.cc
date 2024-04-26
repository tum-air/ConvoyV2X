
#include "MembershipControl.h"
#include "nodes/BaseNode.h"
#include "stack/phy/layer/NRPhyUe.h"
#include "stack/phy/layer/LtePhyEnbD2D.h"
#include "common/binder/Binder.h"

namespace convoy_architecture {

Define_Module(MembershipControl);

MembershipControl::~MembershipControl()
{
    cancelAndDelete(_event_scan_broadcast);
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

void MembershipControl::initialize()
{
    // register to get a notification when position changes
    getParentModule()->subscribe(inet::IMobility::mobilityStateChangedSignal, this);

    // Create primary cluster device
    // and initiate its role as a member for broadcast reception
    _device_pr = createInitClusterDevice(ClusterDeviceType::MEMBER_DEVICE, DeviceInterfaceType::PRIMARY_INTERFACE);

    // Setup primary device to periodically scan for broadcast messages
    _event_scan_broadcast = new omnetpp::cMessage("EventScanBroadcast");
    double scan_duration = par("broadcastScanDuration").doubleValue();
    double scan_interval = par("broadcastScanInterval").doubleValue();
    scan_duration = (scan_duration < scan_interval)? (3.0 * scan_interval) : scan_duration;
    scheduleAfter(scan_duration, _event_scan_broadcast);
}

ClusterDevice* MembershipControl::createInitClusterDevice(ClusterDeviceType device_type, DeviceInterfaceType interface_type) {
    std::string device_name ="unknown";
    int device_index = 0;

    if(device_type == ClusterDeviceType::MEMBER_DEVICE) {
        device_name = "member";
    }
    else if(device_type == ClusterDeviceType::MASTER_DEVICE) {
        device_name = "master";
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
    device->setType(device_type, interface_type);

    // Initialize mobility
    updateLocation(device);

    // Configure and start device
    device->configureAndStart();

    // register to get a notification when broadcast messages arrive
    if(device_type != ClusterDeviceType::MASTER_DEVICE) {
        NRPhyUe *nic = check_and_cast<NRPhyUe *>(device->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        nic->subscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
    }
    else {
        LtePhyEnbD2D *nic = check_and_cast<LtePhyEnbD2D *>(device->getSubmodule("cellularNic")->getSubmodule("phy"));
        nic->subscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
    }

    return device;
}

void MembershipControl::updateLocation(ClusterDevice* device)
{
    double offset_x, offset_y;
    std::string color = "red";
    switch (device->getDeviceType())
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
    std::string event{msg->getName()};
    if(event == std::string{"EventScanBroadcast"})
        handleBroadcastScanEvent();
}

void MembershipControl::handleBroadcastScanEvent() {
    // Check for arrival of broadcast messages
    if(_broadcasts_received.size() > 0) {
        //
    }
    else
        createNewConvoy();
}

void MembershipControl::createNewConvoy() {
    // Create and notify a new convoy
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    int id_convoy{binder->getMaxIdConvoy()};
    binder->notifyConvoy(id_convoy++);
    createNewCluster(id_convoy);
}

void MembershipControl::createNewCluster(int id_convoy) {
    // Set up primary device as a new cluster in the convoy < id_convoy >
    // But first clear up old device info if any
    _device_pr->disconnect();
    _device_pr->remove();
    _device_pr = createInitClusterDevice(ClusterDeviceType::MASTER_DEVICE, DeviceInterfaceType::PRIMARY_INTERFACE);

    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    /*
     * CONTINUE HERE
     * binder->notifyCluster(id_convoy, cluster_id, _manager_id);
     */


    /*
     * _cluster_id = cluster_id;

    // Remove existing cluster manager if present
    removeExistingClusterManager();

    // Create and initialize new cluster manager
    _cluster_role = Role::MANAGER;
    createNewClusterManager();

    // Notify binder
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    binder->notifyCluster(_convoy_id, cluster_id, _manager_id);

    // Set cluster control broadcast parameters
    setClusterBroadcastParameters();

    // Change cluster broadcast subscription
    LtePhyEnbD2D *manager_phy = check_and_cast<LtePhyEnbD2D *>(_manager_module->getSubmodule("cellularNic")->getSubmodule("phy"));
    NRPhyUe *member_phy = check_and_cast<NRPhyUe *>(_member_module->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
    if(member_phy->isSubscribed(LtePhyBase::clusterBroadcastReceivedSignal, this))
        member_phy->unsubscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
    if(!manager_phy->isSubscribed(LtePhyBase::clusterBroadcastReceivedSignal, this))
        manager_phy->subscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
     *
     */
}

const std::vector<Publication>& MembershipControl::readPublishers() const
{
    return _publishers;
}

ClusterDeviceType MembershipControl::clusterMatch(int cluster_address) {
    if(_device_pr != nullptr) {
        if(cluster_address == _device_pr->getAddressOfManager())
            return _device_pr->getDeviceType();
    }
    if(_device_gw != nullptr) {
        if(cluster_address == _device_gw->getAddressOfManager())
            return _device_gw->getDeviceType();
    }
    return(ClusterDeviceType::UNKNOWN_DEVICE);
}

void MembershipControl::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, omnetpp::cObject *)
{
    if (signalID == inet::IMobility::mobilityStateChangedSignal) {
        inet::IMobility *base_mobility = check_and_cast<inet::IMobility*>(obj);
        if(_device_pr != nullptr)
            updateLocation(_device_pr);
        if(_device_gw != nullptr)
            updateLocation(_device_gw);
    }
    else if (signalID == LtePhyBase::clusterBroadcastReceivedSignal)
    {
        ClusterBroadcastNotification *notification = check_and_cast<ClusterBroadcastNotification*>(obj);
        UserControlInfo *msg = notification->_cluster_broadcast_message;

        Broadcast broadcast {
                    msg->getConvoyID(),
                    msg->getClusterID(),
                    msg->getConvoyFull(),
                    msg->getClusterFull(),
                    (ConvoyDirection) msg->getConvoyDirection(),
                    msg->getRssi()
        };
        _broadcasts_received.push_back(broadcast);

        // Update publisher details if received signal strength is above threshold
        if(msg->getRssi() >= par("minRSSIClusterManager").doubleValue()) {
            Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
            for(int i=0; i<msg->getPublisherNodeIDArraySize(); i++) {
                Publication publisher {
                            std::string(msg->getPublisherName(i)),
                            msg->getPublisherNodeID(i),
                            inet::Coord{msg->getPublisherNodeX(i), msg->getPublisherNodeY(i), 0.0},
                            msg->getPublisherFOV(i),
                            (ConvoyDirection) msg->getConvoyDirection(),
                            binder->getNextHop(msg->getPublisherNodeID(i))
                };

                // Replace existing entry if the publisher already exists , else create a new entry
                std::string publisher_name = publisher.name;
                auto it = std::find_if(std::begin(_publishers), std::end(_publishers), [&publisher_name] (Publication const& val) {return val.name == publisher_name;});
                if(it != std::end(_publishers))
                    (*it) = publisher;
                else
                    _publishers.push_back(publisher);
            }
        }
    }
}

} // namespace convoy_architecture
