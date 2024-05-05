
#include "MembershipControl.h"
#include "nodes/BaseNode.h"
#include "stack/phy/layer/NRPhyUe.h"
#include "stack/phy/layer/LtePhyEnbD2D.h"
#include "common/binder/Binder.h"
#include "messages/MemberStatus_m.h"

namespace convoy_architecture {

Define_Module(MembershipControl);

MembershipControl::~MembershipControl()
{
    cancelAndDelete(_event_scan_broadcast);
    _event_scan_broadcast = nullptr;
    cancelAndDelete(_event_publisher_update);
    _event_publisher_update = nullptr;
    cancelAndDelete(_event_member_report);
    _event_member_report = nullptr;

    // Disconnect and remove associated cluster devices
    if (_device_pr != nullptr) {
        if(_device_pr->isModule()) {
            _device_pr->disconnect();
            _device_pr->remove();
            _device_pr->deleteModule();
        }
    }
    if (_device_gw != nullptr) {
        if(_device_gw->isModule()) {
            _device_gw->disconnect();
            _device_gw->remove();
            _device_gw->deleteModule();
        }
    }
}

void MembershipControl::initialize()
{
    // register to get a notification when position changes
    getParentModule()->subscribe(inet::IMobility::mobilityStateChangedSignal, this);

    // Create primary and gateway cluster devices
    // and initiate for broadcast reception
    _device_pr = createInitClusterDevice(ClusterDeviceType::MEMBER_DEVICE, DeviceInterfaceType::PRIMARY_INTERFACE);
    _device_gw = createInitClusterDevice(ClusterDeviceType::GATEWAY_DEVICE, DeviceInterfaceType::GATEWAY_INTERFACE);

    _event_scan_broadcast = new omnetpp::cMessage("EventScanBroadcast");
    _event_publisher_update = new omnetpp::cMessage("EventPublisherUpdate");
    _event_member_report = new omnetpp::cMessage("EventMemberReport");

    // Setup primary device to periodically scan for broadcast messages
    double scan_duration = par("broadcastScanDuration").doubleValue();
    double scan_interval = par("broadcastScanInterval").doubleValue();
    scan_duration = (scan_duration < scan_interval)? (3.0 * scan_interval) : scan_duration;
    scheduleAfter(scan_duration, _event_scan_broadcast);

    WATCH(_control_state);
    WATCH(_role);
}

ClusterDevice* MembershipControl::createInitClusterDevice(ClusterDeviceType device_type, DeviceInterfaceType interface_type, int id_convoy) {
    std::string device_name ="unknown";
    int device_index = 0;

    if(device_type == ClusterDeviceType::MEMBER_DEVICE)
        device_name = "member";
    else if(device_type == ClusterDeviceType::MASTER_DEVICE)
        device_name = "master";
    else
        device_name = "gateway";

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
    device->configureAndStart(par("convoyDirection").intValue(), id_convoy);

    // Connect device to base node
    if(device_type == ClusterDeviceType::GATEWAY_DEVICE)
        device->connectGatewayDeviceTo(getParentModule());
    else
        device->connectPrimaryDeviceTo(getParentModule());

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
        color = "blue";
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
    if(msg->isSelfMessage()) {
        if(event == std::string{"EventScanBroadcast"})
            handleBroadcastScanEvent();
        else if((event == std::string{"EventPublisherUpdate"}) && (_device_pr->getDeviceType() == ClusterDeviceType::MASTER_DEVICE)) {
            updatePublishers();
            scheduleAfter(par("publisherUpdateInterval").doubleValue(), _event_publisher_update);
        }
        else if(event == std::string{"EventMemberReport"}) {
            sendMemberReport();
            scheduleAfter(par("memberReportUpdateRate").doubleValue(), _event_member_report);
        }

    }
    else if(msg->arrivedOn("in")) {
        if(event == std::string{"ConvoyOrchestration"}) {
            handleOrchestrationMessage(check_and_cast<ConvoyControlService *>(msg));
        }
    }

}

void MembershipControl::handleBroadcastScanEvent() {
    // Check for arrival of broadcast messages
    if(_broadcasts_received.size() > 0) {
        // Extract broadcasts from those clusters which are moving in the same direction
        // and whose signal strengths are greater than minRSSI
        ConvoyDirection direction = (ConvoyDirection) par("convoyDirection").intValue();
        double min_rssi = par("minRSSIClusterManager").doubleValue();
        std::vector<Broadcast> qualified_broadcasts {};
        std::copy_if(std::begin(_broadcasts_received), std::end(_broadcasts_received), std::back_inserter(qualified_broadcasts),
                [&direction, &min_rssi] (Broadcast const& broadcast) {
            return (broadcast.direction == direction) && (broadcast.rssi >= min_rssi);
        });

        if(qualified_broadcasts.size() > 0)
        {
            // Sort the qualified in descending order of received signal strength
            std::sort(std::begin(qualified_broadcasts), std::end(qualified_broadcasts),
                    [] (Broadcast const& lhs, Broadcast const& rhs) {
                return lhs.rssi < rhs.rssi;
            });

            // Iterate through the sorted list starting with the max rssi candidate
            bool joined_existing_convoy = false;
            for(auto candidate : qualified_broadcasts) {
                if(!candidate.full_convoy) {
                    if(!candidate.full_cluster) {
                        joinNewCluster(candidate.id_convoy, candidate.id_cluster, DeviceInterfaceType::PRIMARY_INTERFACE, candidate.rssi);
                    }
                    else {
                        createNewCluster(candidate.id_convoy);
                    }
                    joined_existing_convoy = true;
                    break;
                }
            }
            if(!joined_existing_convoy)
                createNewConvoy();
        }
        else
            createNewConvoy();
    }
    else
        createNewConvoy();

    scheduleAfter(par("publisherUpdateInterval").doubleValue(), _event_publisher_update);
    omnetpp::simtime_t current_time = omnetpp::simTime();
    double start_time = par("memberReportStartTime").doubleValue();
    omnetpp::simtime_t trigger_time = (current_time < start_time)? start_time : current_time;
    scheduleAt(trigger_time, _event_member_report);
}

void MembershipControl::createNewConvoy() {
    // Create and notify a new convoy
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    int id_convoy{binder->getMaxIdConvoy()};
    binder->registerConvoy(++id_convoy);
    createNewCluster(id_convoy);
}

void MembershipControl::createNewCluster(int id_convoy) {
    // Set up primary device as a new cluster in the convoy < id_convoy >
    // But first clear up old device info if any
    _device_pr->disconnect();
    _device_pr->remove();
    _device_pr->deleteModule();
    _device_pr = createInitClusterDevice(ClusterDeviceType::MASTER_DEVICE, DeviceInterfaceType::PRIMARY_INTERFACE, id_convoy);

    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    binder->registerCluster(id_convoy, _device_pr->getAddressOfManager());

    // Update publisher list in cluster device for broadcast
    updatePublishers();

    _role = Role::MANAGER;
    _control_state = MemberControlState::INITIALIZED;
}

void MembershipControl::joinNewCluster(int id_convoy, int id_cluster, DeviceInterfaceType interface_type, double rssi) {
    if(interface_type == DeviceInterfaceType::GATEWAY_INTERFACE) {
        _device_gw->joinNewCluster(id_convoy, id_cluster, rssi);
        _role = Role::GATEWAY;
    }
    else {
        _device_pr->joinNewCluster(id_convoy, id_cluster, rssi);
        _role = (_role == Role::GATEWAY)? Role::GATEWAY : Role::MEMBER;
        _control_state = MemberControlState::INITIALIZED;
    }
}

void MembershipControl::detachFromCluster(DeviceInterfaceType interface_type) {
    if(interface_type == DeviceInterfaceType::GATEWAY_INTERFACE) {
        if(_device_gw->isModule() && (_device_gw->getAddressOfManager() != 0))
            _device_gw->detachFromCluster();
        _role = Role::MEMBER;
    }
    else {
        if(_device_pr->isModule() && (_device_pr->getAddressOfManager() != 0)) {
            if(_device_pr->getDeviceType() == ClusterDeviceType::MASTER_DEVICE) {
                Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
                binder->deRegisterCluster(_device_pr->getAddressOfConvoy(), _device_pr->getAddressOfManager());
            }
            _device_pr->detachFromCluster();
        }
        _role = Role::UNDEFINED;
        _control_state = MemberControlState::INITIALIZING;
    }
}

void MembershipControl::updatePublishers()
{
    // If the parent module is of station type RSU, include the cluster device corresponding to the parent node in the publisher list
    BaseNode* base_node = check_and_cast<BaseNode*>(getParentModule());
    if(base_node->getStationType() == StationType::RSU)
    {
        Publication publisher {
                    std::string(base_node->getFullName()),
                    _device_pr->getAddress(),
                    base_node->getCurrentLocation(),
                    base_node->par("detectorFovLimitRange").doubleValue(),
                    (ConvoyDirection) par("convoyDirection").intValue(),
                    _device_pr->getAddressOfManager()
        };

        auto it = std::find_if(std::begin(_publishers), std::end(_publishers), [&publisher] (Publication const& val) {return val.name == publisher.name;});
        if(it != std::end(_publishers))
            (*it) = publisher;
        else
            _publishers.push_back(publisher);
    }
    _device_pr->updateBroadcastInfo(_publishers);
}

void MembershipControl::sendMemberReport()
{
    BaseNode* base_node = check_and_cast<BaseNode*>(getParentModule());
    inet::Coord location = base_node->getCurrentLocation();
    MemberStatus *msg = new MemberStatus("MemberReport");
    msg->setTimestamp((uint64_t )omnetpp::simTime().raw());
    msg->setCcs_report_name(getParentModule()->getFullName());
    msg->setCcs_report_direction(par("convoyDirection").intValue());
    msg->setCcs_report_id_convoy(_device_pr->getAddressOfConvoy());
    msg->setCcs_report_id_gnb(_device_pr->getAddressOfManager());
    msg->setCcs_report_id_ue(_device_pr->getAddress());
    msg->setCcs_report_id_gnb(_device_gw->getAddressOfManager());
    msg->setCcs_report_id_ue(_device_gw->getAddress());
    msg->setCcs_report_role((int)_role);
    msg->setCcs_report_position_x(location.x);
    msg->setCcs_report_position_y(location.y);
    msg->setCcs_report_position_z(location.z);
    msg->setCcs_report_txp(_device_pr->getTxp());
    msg->setCcs_report_txp_gw(_device_gw->getTxp());
    send(msg, "out");
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
        // Replace existing entry if the cluster already exists , else create a new entry
        auto itr = std::find_if(std::begin(_broadcasts_received), std::end(_broadcasts_received), [&broadcast] (Broadcast const& val) {return val.id_cluster == broadcast.id_cluster;});
        if(itr != std::end(_broadcasts_received))
            (*itr) = broadcast;
        else
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

void MembershipControl::handleOrchestrationMessage(ConvoyControlService *msg) {
    double txp = msg->getTx_power();
    double txp_gw = msg->getTx_power_gw();
    Role role = (Role) msg->getCluster_role();
    int id_convoy = msg->getConvoy_id();
    int id_cluster = msg->getCluster_id();
    int id_cluster_gw = msg->getCluster_id_gw();

    // Enforce role and membership changes if required
    if(id_convoy != _device_pr->getAddressOfConvoy()) {
        // changing to new convoy
        _device_pr->detachFromCluster();
        _device_gw->detachFromCluster();
        Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
        if(id_convoy > binder->getMaxIdConvoy())
            createNewConvoy();
        changeClusterMembership(role, id_convoy, id_cluster, id_cluster_gw);
    }
    else if(role != _role) {
        _device_pr->detachFromCluster();
        _device_gw->detachFromCluster();
        changeClusterMembership(role, id_convoy, id_cluster, id_cluster_gw);
    }
    else if((id_cluster != _device_pr->getAddressOfManager()) || (id_cluster_gw != _device_gw->getAddressOfManager()))
        changeClusterMembership(role, id_convoy, id_cluster, id_cluster_gw);

    // Enforce txp limitations
    if(_device_pr->isModule())
        _device_pr->setTxp(txp);
    if(_device_gw->isModule())
        _device_gw->setTxp(txp_gw);
}

void MembershipControl::changeClusterMembership(Role role, int id_convoy, int id_cluster, int id_cluster_gw) {
    Role previous_role {_role};
    if(role == Role::MANAGER)
        createNewCluster(id_convoy);
    else {
        if(role == Role::GATEWAY) {
            double rssi{0.0};
            auto itr = std::find_if(std::begin(_broadcasts_received), std::end(_broadcasts_received), [&id_cluster_gw] (Broadcast const& val) {return val.id_cluster == id_cluster_gw;});
            if(itr != std::end(_broadcasts_received))
                rssi = (*itr).rssi;
            joinNewCluster(id_convoy, id_cluster_gw, DeviceInterfaceType::GATEWAY_INTERFACE, rssi);
        }
        if(role == Role::MEMBER || role == Role::GATEWAY) {
            if(previous_role == Role::MANAGER) {
                _device_pr->disconnect();
                _device_pr->remove();
                _device_pr->deleteModule();
                _device_pr = createInitClusterDevice(ClusterDeviceType::MEMBER_DEVICE, DeviceInterfaceType::PRIMARY_INTERFACE, id_convoy);
            }
            double rssi{0.0};
            auto itr = std::find_if(std::begin(_broadcasts_received), std::end(_broadcasts_received), [&id_cluster] (Broadcast const& val) {return val.id_cluster == id_cluster;});
            if(itr != std::end(_broadcasts_received))
                rssi = (*itr).rssi;
            joinNewCluster(id_convoy, id_cluster, DeviceInterfaceType::PRIMARY_INTERFACE, rssi);
        }
    }
}
} // namespace convoy_architecture
