//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "ConvoyControl.h"
#include "stack/phy/layer/NRPhyUe.h"
#include "stack/phy/layer/LtePhyEnbD2D.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/common/InitStages.h"
#include "common/binder/Binder.h"
#include "veins_inet/VeinsInetMobility.h"
#include "inet/mobility/static/StationaryMobility.h"
#include "nodes/WrapperNode.h"

namespace convoy_architecture {

Define_Module(ConvoyControl);

ConvoyControl::ConvoyControl()
{
    _manager_module = nullptr;
    _member_module = nullptr;
    _gateway_module = nullptr;
    _cluster_broadcast_scan_trigger = nullptr;
    _cluster_broadcast_scan_timeout = nullptr;
    _localization_complete_trigger = nullptr;
    _publisher_list_update_trigger = nullptr;
    _run_mode_update_trigger = nullptr;
}

ConvoyControl::~ConvoyControl()
{
    cancelAndDelete(_cluster_broadcast_scan_trigger);
    cancelAndDelete(_cluster_broadcast_scan_timeout);
    cancelAndDelete(_localization_complete_trigger);
    cancelAndDelete(_publisher_list_update_trigger);
    cancelAndDelete(_run_mode_update_trigger);

    // Delete associated cluster devices
    if(_manager_module != nullptr)
    {
        if(_manager_module->isModule())
        {
            removeConnectionsToWrapperNode(_manager_module);
            _manager_module->deleteModule();

            // Inform binder
            Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
            binder->deNotifyCluster(_manager_id);
        }
    }
    if(_member_module != nullptr)
    {
        if(_member_module->isModule())
        {
            removeConnectionsToWrapperNode(_member_module);
            _member_module->deleteModule();
        }
    }
    if(_gateway_module != nullptr)
    {
        if(_gateway_module->isModule())
        {
            removeConnectionsToWrapperNode(_gateway_module);
            _gateway_module->deleteModule();
        }
    }
}

void ConvoyControl::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - ConvoyControl::initialize(): " << "Initializing convoy control agent " << std::endl;

    _convoy_direction = static_cast<ConvoyDirection>(par("convoyDirection").intValue());
    _cluster_broadcast_scan_duration = par("clusterBroadcastScanDuration").doubleValue();
    _cluster_broadcast_scan_interval = par("clusterBroadcastScanInterval").doubleValue();
    _cluster_init_localization_interval = par("clusterInitLocalizationInterval").doubleValue();
    _ccs_broadcast_interval = par("ccsBroadcastInterval").doubleValue();
    _publisher_list_update_interval = par("publisherListUpdateInterval").doubleValue();
    _agent_run_mode_update_interval = par("agentRunModeUpdateInterval").doubleValue();
    _max_n_clusters = par("maxNClusters").intValue();
    _station_type = (StationType) par("stationType").intValue();

    _cluster_broadcast_scan_trigger = new omnetpp::cMessage("broadcastScanTrigger");
    _cluster_broadcast_scan_timeout = new omnetpp::cMessage("broadcastScanTimeout");
    _localization_complete_trigger = new omnetpp::cMessage("localizationCompleted");
    _publisher_list_update_trigger  = new omnetpp::cMessage("publisherListUpdateTrigger");
    _run_mode_update_trigger = new omnetpp::cMessage("runModeUpdateTrigger");


    // register to get a notification when position changes
    getParentModule()->subscribe(inet::IMobility::mobilityStateChangedSignal, this);

    // Initialize cluster member for broadcast reception
    _cluster_role = Role::MEMBER;
    createNewClusterMember();
    initCluster();

    EV_INFO << current_time <<" - ConvoyControl::initialize(): " << "Initialized convoy control agent for station " << this->getParentModule()->getFullName() << std::endl;

    WATCH(_agent_status);
    WATCH(_convoy_direction);
}

/*

void ConvoyControl::setAsConvoyGateway()
{
    //
}

void ConvoyControl::attachClusterGateway()
{
    //
}

*/

void ConvoyControl::createNewClusterMember()
{
    std::string module_name = "member";
    int module_index = 0;

    // Create submodule vector first, if it doesn't already exist
    if(!getSystemModule()->hasSubmoduleVector(module_name.c_str()))
        getSystemModule()->addSubmoduleVector(module_name.c_str(), 1);
    else
    {
        module_index = getSystemModule()->getSubmoduleVectorSize(module_name.c_str());
        getSystemModule()->setSubmoduleVectorSize(module_name.c_str(), (module_index + 1));
    }

    // Create new cluster member module
    omnetpp::cModuleType *module_type = omnetpp::cModuleType::get("convoy_architecture.nodes.ClusterMember");
    _member_module = module_type->create(module_name.c_str(), getSystemModule(), module_index);
    _member_module->setDisplayName("");

    // Initialize mobility
    setNewLocation(_member_module, Role::MEMBER);

    // Finalize module
    _member_module->finalizeParameters();
    _member_module->buildInside();

    // Start module
    _member_module->scheduleStart(omnetpp::simTime());
    _member_module->callInitialize();

    // Use macNodeId assigned by binder as member id
    _member_id = _member_module->par("macNodeId");

    // register to get a notification when broadcast messages arrive
    NRPhyUe *member_phy = check_and_cast<NRPhyUe *>(_member_module->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
    member_phy->subscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
}

void ConvoyControl::createNewClusterGateway()
{
    //
}

void ConvoyControl::setNewLocation(omnetpp::cModule* module, Role role)
{
    double offset_x, offset_y;
    switch (role)
    {
    case MANAGER:
        offset_x = 0.0;
        offset_y = -1.0;
        break;
    case GATEWAY:
        offset_x = -2.0;
        offset_y = -1.0;
        break;
    default:
        offset_x = 2.0;
        offset_y = -1.0;
        break;
    }

    // Get current location of wrapper node
    WrapperNode* wrapper_node = check_and_cast<WrapperNode*>(getParentModule());
    inet::Coord location = wrapper_node->getCurrentLocation();

    // Update display string
    omnetpp::cDisplayString display_string = module->getDisplayString();
    display_string.setTagArg("p", 0, (location.x + offset_x));
    display_string.setTagArg("p", 1, (location.y + offset_y));
    module->setDisplayString(display_string.str());
}

void ConvoyControl::initCluster()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    _agent_status = STATUS_INIT_SCANNING;

    if(_cluster_broadcast_scan_duration < _cluster_broadcast_scan_interval)
    {
        EV_INFO << current_time <<" - ConvoyControl::initCluster(): " << "scan duration is smaller than scan interval, setting duration = 3 x interval " << std::endl;
        _cluster_broadcast_scan_duration = 3 * _cluster_broadcast_scan_interval;
    }

    scheduleAfter(_cluster_broadcast_scan_duration, _cluster_broadcast_scan_timeout);
}

void ConvoyControl::handleMessage(omnetpp::cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (msg == _publisher_list_update_trigger)
        {
            // Set cluster control broadcast parameters
            setClusterBroadcastParameters();
            scheduleAfter(_publisher_list_update_interval, _publisher_list_update_trigger);
        }
        else if (msg == _run_mode_update_trigger)
            runModeUpdates();
        else if ((_agent_status == STATUS_INIT_SCANNING) || (_agent_status == STATUS_INIT_LOCALIZING))
            initModeUpdates(msg);
    }
    else if(msg->arrivedOn("in"))
    {
        ConvoyControlService *ccs_msg = check_and_cast<ConvoyControlService *>(msg);
        receiveCCSMessage(ccs_msg);
        delete msg;
    }
}

void ConvoyControl::initModeUpdates(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    switch (_agent_status)
    {
        case STATUS_INIT_SCANNING:
            if (msg == _cluster_broadcast_scan_timeout)
            {
                bool clusterBroadcastReceived = (_broadcast_rx_cluster_id.size() > 0);

                if(clusterBroadcastReceived)
                    EV_INFO << current_time <<" - ConvoyControl::initModeUpdates(): " << "Broadcast signal received, proceeding with initial setup " << std::endl;
                else
                    EV_INFO << current_time <<" - ConvoyControl::initModeUpdates(): " << "Timeout while scanning for broadcast signal" << std::endl;

                // Finish setting up cluster and clear local broadcast record for next read
                setupInitialRole(clusterBroadcastReceived);
                clearLocalBroadcastRecord();

                _agent_status = STATUS_INIT_LOCALIZING;
                cancelEvent(_cluster_broadcast_scan_timeout);
            }
            if (_agent_status == STATUS_INIT_LOCALIZING)
            {
                scheduleAfter(_cluster_init_localization_interval, _localization_complete_trigger);
                scheduleAfter(_publisher_list_update_interval, _publisher_list_update_trigger);
            }
            break;
        case STATUS_INIT_LOCALIZING:
            _agent_status = STATUS_INIT_RUN;
            scheduleAt(current_time, _run_mode_update_trigger);
            break;
        default:
            break;
    }
}

/*
bool ConvoyControl::newClusterBroadcastReceived()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - ConvoyControl::newClusterBroadcastReceived(): " << "Checking for cluster broadcast signals" << std::endl;

    // Check for broadcast record
    bool broadcast_record_available = (_broadcast_rx_cluster_id.size() > 0);
    if(_cluster_role != Role::MANAGER)
    {
        NRPhyUe *cluster_member_phy = check_and_cast<NRPhyUe *>(_member_module->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        _cl_brd_convoy_id = cluster_member_phy->readBroadcastConvoyId();
        _cl_brd_cluster_id = cluster_member_phy->readBroadcastClusterId();
        _cl_brd_cluster_direction = cluster_member_phy->readBroadcastClusterDirection();
        _cl_brd_convoy_full = cluster_member_phy->readBroadcastConvoyFull();
        _cl_brd_cluster_full = cluster_member_phy->readBroadcastClusterFull();
        _cl_brd_cluster_rssi = cluster_member_phy->readBroadcastClusterRssi();
        _cl_brd_mac_cell_id = cluster_member_phy->readBroadcastMasterId();


    }
    // Clear broadcast record for next read
    clearLocalBroadcastRecord();
    return (_cl_brd_cluster_id.size() > 0);
}
*/
void ConvoyControl::clearLocalBroadcastRecord()
{
    _broadcast_rx_cluster_id.clear();
    _broadcast_rx_convoy_id.clear();
    _broadcast_rx_cluster_full.clear();
    _broadcast_rx_convoy_full.clear();
    _broadcast_rx_cluster_direction.clear();
    _broadcast_rx_cluster_rssi.clear();
}

void ConvoyControl::setupInitialRole(bool broadcast_received)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Use new convoy id based on binder record
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    int convoy_id = binder->getMaxIdConvoy() + 1;

    if (broadcast_received)
    {
        // Select nearest cluster moving in the same direction based on signal strength - check against minimum threshold
        // If no cluster is found moving in the same direction continue setting up initial convoy
        std::map<int, double>::iterator it;
        int ref_id_max_rssi = 0;
        double max_rssi = par("minRSSIClusterManager").doubleValue()-1000.0;
        for(it = _broadcast_rx_cluster_rssi.begin(); it != _broadcast_rx_cluster_rssi.end(); ++it)
        {
            if(it->second >= max_rssi)
            {
                max_rssi = it->second;
                ref_id_max_rssi = it->first;
            }
        }
        if(max_rssi >= par("minRSSIClusterManager").doubleValue())
        {
            if(_broadcast_rx_cluster_direction[ref_id_max_rssi] == (int) _convoy_direction)
            {
                if(!_broadcast_rx_convoy_full[ref_id_max_rssi])
                {
                    // Join the convoy
                    EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): " << "Joining convoy " << _broadcast_rx_convoy_id[ref_id_max_rssi] << std::endl;
                    convoy_id = _broadcast_rx_convoy_id[ref_id_max_rssi];
                    setAsConvoyMember(convoy_id);

                    if(_broadcast_rx_cluster_full[ref_id_max_rssi])
                    {
                        EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): " << "Cluster " << ref_id_max_rssi << " full, setting up new cluster" << std::endl;

                        // Set up new cluster and configure node as cluster manager
                        int cluster_id = binder->getMaxIdCluster(convoy_id) + 1;
                        setAsClusterManager(cluster_id);
                    }
                    else
                    {
                        EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): " << "Joining cluster " << ref_id_max_rssi << std::endl;

                        // Configure node as cluster member
                        _cluster_id = binder->getClusterId(convoy_id, (unsigned short) ref_id_max_rssi);
                        setAsClusterMember((unsigned short) ref_id_max_rssi, _broadcast_rx_cluster_rssi[ref_id_max_rssi]);
                    }

                    return;
                }

                EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): " << "Convoy corresponding to max cluster rssi " << _broadcast_rx_convoy_id[ref_id_max_rssi] << " is full" <<  std::endl;
                EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): " << "Setting up new convoy with id: " << convoy_id << std::endl;
                // Set up new convoy and configure node as convoy manager
                // Set up new cluster and configure node as cluster manager
                setAsConvoyManager(convoy_id);
                int cluster_id = binder->getMaxIdCluster(convoy_id) + 1;
                setAsClusterManager(cluster_id);
                return;
            }
            else
            {
                EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): cluster with max rssi is not in same direction, checking next best candidate " << std::endl;
                _broadcast_rx_cluster_rssi[ref_id_max_rssi] = par("minRSSIClusterManager").doubleValue() - 1.0; // this element will not qualify for next check
                setupInitialRole(true);
            }
        }
        else
        {
            EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): cluster manager with max rssi of " << max_rssi << " is below minRSSIClusterManager threshold " << std::endl;
            EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): " << "Setting up new convoy with id: " << convoy_id << std::endl;
            // Set up new convoy and configure node as convoy manager
            // Set up new cluster and configure node as cluster manager
            setAsConvoyManager(convoy_id);
            int cluster_id = binder->getMaxIdCluster(convoy_id) + 1;
            setAsClusterManager(cluster_id);
            return;
        }
    }
    else
    {
        EV_INFO << current_time <<" - ConvoyControl::setupInitialRole(): " << "Setting up new convoy with id: " << convoy_id << std::endl;

        // Set up new convoy and configure node as convoy manager
        // Set up new cluster and configure node as cluster manager
        setAsConvoyManager(convoy_id);
        int cluster_id = binder->getMaxIdCluster(convoy_id) + 1;
        setAsClusterManager(cluster_id);
    }
}

void ConvoyControl::setAsConvoyMember(int convoy_id)
{
    // Set up convoy and configure node as convoy member
    _convoy_id = convoy_id;
    _convoy_role = Role::MEMBER;
}

void ConvoyControl::setAsClusterManager(int cluster_id)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    _cluster_id = cluster_id;

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

    EV_INFO << current_time <<" - ConvoyControl::setAsClusterManager(): " << " cluster manager setup completed" << std::endl;
}

void ConvoyControl::removeExistingClusterManager()
{
    if(_manager_module != nullptr)
    {
        if(_manager_module->isModule())
        {
            removeConnectionsToWrapperNode(_manager_module);
            _manager_module->deleteModule();

            // Inform binder
            Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
            binder->deNotifyCluster(_manager_id);
        }
    }
}

void ConvoyControl::createNewClusterManager()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string module_name = "manager";
    int module_index = 0;

    // Create submodule vector first, if it doesn't already exist
    if(!getSystemModule()->hasSubmoduleVector(module_name.c_str()))
    {
        EV_INFO << current_time <<" - ConvoyControl::createNewClusterManager(): " << " creating submodule vector for manager nodes " << std::endl;
        getSystemModule()->addSubmoduleVector(module_name.c_str(), 1);
    }
    else
    {
        module_index = getSystemModule()->getSubmoduleVectorSize(module_name.c_str());
        EV_INFO << current_time <<" - ConvoyControl::createNewClusterManager(): " << " resizing manager submodule vector size to " << (module_index + 1) << std::endl;
        getSystemModule()->setSubmoduleVectorSize(module_name.c_str(), (module_index + 1));
    }

    EV_INFO << current_time <<" - ConvoyControl::createNewClusterManager(): " << " creating cluster manager at index " << module_index << std::endl;

    // Create new cluster member module
    omnetpp::cModuleType *module_type = omnetpp::cModuleType::get("convoy_architecture.nodes.ClusterManager");
    _manager_module = module_type->create(module_name.c_str(), getSystemModule(), module_index);
    _manager_module->setDisplayName("");

    // Initialize mobility
    setNewLocation(_manager_module, Role::MANAGER);

    // Finalize module
    _manager_module->par("convoyID") = _convoy_id;
    _manager_module->par("convoyFull") = false;
    _manager_module->par("clusterFull") = false;
    _manager_module->par("convoyDirection") = (int) _convoy_direction;
    _manager_module->finalizeParameters();
    _manager_module->buildInside();

    // Start module
    _manager_module->scheduleStart(omnetpp::simTime());
    _manager_module->callInitialize();

    // Use macNodeId assigned by binder as member id
    _manager_id = _manager_module->par("macNodeId");

    // Remove old connections - member module
    removeConnectionsToWrapperNode(_member_module);

    // Setup connections to all gates of wrapper node
    connectToWrapperNode(_manager_module);
}

void ConvoyControl::setAsConvoyManager(int convoy_id)
{
    // Set up convoy and configure node as convoy manager
    _convoy_id = convoy_id;
    _convoy_role = Role::MANAGER;

    // Notify binder
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    binder->notifyConvoy(convoy_id);
}

void ConvoyControl::setAsClusterMember(unsigned short cluster_id, double cluster_rssi)
{
    // Set up cluster and configure node as cluster member
    _manager_id = cluster_id;
    _cluster_role = Role::MEMBER;

    // Join the target cluster
    NRPhyUe *member_phy = check_and_cast<NRPhyUe *>(_member_module->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
    WrapperNode* wrapper_node = check_and_cast<WrapperNode*>(getParentModule());
    bool is_rsu = (wrapper_node->getStationType() == StationType::RSU);
    member_phy->attachToCluster(cluster_id, cluster_rssi, is_rsu);

    _member_id = _member_module->par("nrMacNodeId");

    // Remove old connections - manager module
    removeConnectionsToWrapperNode(_manager_module);

    // Setup new connections to all gates of wrapper node
    connectToWrapperNode(_member_module);

    // Set cluster control broadcast parameters
    setClusterBroadcastParameters();

    // Change cluster broadcast subscription
    if(_manager_module != nullptr)
    {
        LtePhyEnbD2D *manager_phy = check_and_cast<LtePhyEnbD2D *>(_manager_module->getSubmodule("cellularNic")->getSubmodule("phy"));
        if(manager_phy->isSubscribed(LtePhyBase::clusterBroadcastReceivedSignal, this))
            manager_phy->unsubscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
    }
    if(!member_phy->isSubscribed(LtePhyBase::clusterBroadcastReceivedSignal, this))
    {
        member_phy->subscribe(LtePhyBase::clusterBroadcastReceivedSignal, this);
    }

}

void ConvoyControl::receiveSignal(omnetpp::cComponent *source, omnetpp::simsignal_t signalID, omnetpp::cObject *obj, omnetpp::cObject *)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    if (signalID == inet::IMobility::mobilityStateChangedSignal)
    {
        inet::IMobility *wrapper_mobility = check_and_cast<inet::IMobility*>(obj);
        EV_INFO << current_time << " - ConvoyControl::receiveSignal(): wrapper node position changed to " << wrapper_mobility->getCurrentPosition().x << std::endl;

        // Update positions of cluster devices
        if(_manager_module)
            setNewLocation(_manager_module, Role::MANAGER);
        if(_member_module)
            setNewLocation(_member_module, Role::MEMBER);
        if(_gateway_module)
            setNewLocation(_gateway_module, Role::GATEWAY);
    }
    else if (signalID == LtePhyBase::clusterBroadcastReceivedSignal)
    {
        ClusterBroadcastNotification *notification = check_and_cast<ClusterBroadcastNotification*>(obj);
        UserControlInfo *msg = notification->_cluster_broadcast_message;
        EV_INFO << current_time << " - ConvoyControl::receiveSignal(): cluster control message received from " << msg->getClusterID()
                << ", with signal strength: " << msg->getRssi() << std::endl;

        // Cluster signals
        _broadcast_rx_cluster_id.insert(msg->getClusterID());
        _broadcast_rx_convoy_id.insert(std::make_pair(msg->getClusterID(), msg->getConvoyID()));
        _broadcast_rx_cluster_full.insert(std::make_pair(msg->getClusterID(), msg->getClusterFull()));
        _broadcast_rx_convoy_full.insert(std::make_pair(msg->getClusterID(), msg->getConvoyFull()));
        _broadcast_rx_cluster_direction.insert(std::make_pair(msg->getClusterID(), msg->getConvoyDirection()));
        _broadcast_rx_cluster_rssi.insert(std::make_pair(msg->getClusterID(), msg->getRssi()));

        for (auto id: _broadcast_rx_cluster_id)
        {
            EV_INFO << current_time << " - ConvoyControl::receiveSignal(): current broadcast record for cluster " << id << ": "
                    << "convoy_id=" << _broadcast_rx_convoy_id[id] <<  ", "
                    << "cluster_full=" << _broadcast_rx_cluster_full[id] <<  ", "
                    << "convoy_full=" << _broadcast_rx_convoy_full[id] << ", "
                    << "cluster_direction=" << _broadcast_rx_cluster_direction[id] << ", "
                    << "convoy_rssi=" << _broadcast_rx_cluster_rssi[id] << std::endl;
        }

        // Update publisher details if received signal strength is above threshold
        if(msg->getRssi() >= par("minRSSIClusterManager").doubleValue())
        {
            for(int i=0; i<msg->getPublisherNodeIDArraySize(); i++)
            {
                _broadcast_rx_publisher_id.insert(msg->getPublisherNodeID(i));
                _broadcast_rx_publisher_x.insert(std::make_pair(msg->getPublisherNodeID(i), msg->getPublisherNodeX(i)));
                _broadcast_rx_publisher_y.insert(std::make_pair(msg->getPublisherNodeID(i), msg->getPublisherNodeY(i)));
                _broadcast_rx_publisher_fov.insert(std::make_pair(msg->getPublisherNodeID(i), msg->getPublisherFOV(i)));
                _broadcast_rx_publisher_direction.insert(std::make_pair(msg->getPublisherNodeID(i), msg->getPublisherDirection(i)));
            }
        }

        for (auto id: _broadcast_rx_publisher_id)
        {
            EV_INFO << current_time << " - ConvoyControl::receiveSignal(): current record for publisher " << id << ": "
                    << "x=" << _broadcast_rx_publisher_x[id] <<  ", "
                    << "y=" << _broadcast_rx_publisher_y[id] <<  ", "
                    << "fov=" << _broadcast_rx_publisher_fov[id] << ", "
                    << "dir=" << _broadcast_rx_publisher_direction[id] << std::endl;
        }
    }
}

void ConvoyControl::connectToWrapperNode(omnetpp::cModule* cluster_module)
{
    omnetpp::cGate *wrapper_gate, *cluster_gate;

    // AppDtwinPub
    wrapper_gate = getParentModule()->gate("outAppDtwinPub");
    cluster_gate = cluster_module->gate("fromAppDtwinPub");
    wrapper_gate->connectTo(cluster_gate);
    wrapper_gate = getParentModule()->gate("inAppDtwinPub");
    cluster_gate = cluster_module->gate("toAppDtwinPub");
    cluster_gate->connectTo(wrapper_gate);

    // AppDtwinSub
    wrapper_gate = getParentModule()->gate("outAppDtwinSub");
    cluster_gate = cluster_module->gate("fromAppDtwinSub");
    wrapper_gate->connectTo(cluster_gate);
    wrapper_gate = getParentModule()->gate("inAppDtwinSub");
    cluster_gate = cluster_module->gate("toAppDtwinSub");
    cluster_gate->connectTo(wrapper_gate);

    // AppCoopMan
    wrapper_gate = getParentModule()->gate("outAppCoopMan");
    cluster_gate = cluster_module->gate("fromAppCoopMan");
    wrapper_gate->connectTo(cluster_gate);
    wrapper_gate = getParentModule()->gate("inAppCoopMan");
    cluster_gate = cluster_module->gate("toAppCoopMan");
    cluster_gate->connectTo(wrapper_gate);
}

void ConvoyControl::removeConnectionsToWrapperNode(omnetpp::cModule* cluster_module)
{
    omnetpp::cGate *cluster_gate;
    if(cluster_module)
    {
        // AppDtwinPub
        cluster_gate = cluster_module->gate("fromAppDtwinPub");
        if(cluster_gate->isConnected())
            cluster_gate->disconnect();
        cluster_gate = cluster_module->gate("toAppDtwinPub");
        if(cluster_gate->isConnected())
            cluster_gate->disconnect();

        // AppDtwinSub
        cluster_gate = cluster_module->gate("fromAppDtwinSub");
        if(cluster_gate->isConnected())
            cluster_gate->disconnect();
        cluster_gate = cluster_module->gate("toAppDtwinSub");
        if(cluster_gate->isConnected())
            cluster_gate->disconnect();

        // AppCoopMan
        cluster_gate = cluster_module->gate("fromAppCoopMan");
        if(cluster_gate->isConnected())
            cluster_gate->disconnect();
        cluster_gate = cluster_module->gate("toAppCoopMan");
        if(cluster_gate->isConnected())
            cluster_gate->disconnect();
    }
}
void ConvoyControl::getPublisherNodeID(std::set<int> &publisher_node_id)
{
    publisher_node_id.clear();
    publisher_node_id = _broadcast_rx_publisher_id;
}
void ConvoyControl::getPublisherNodeX(std::map<int, double> &publisher_node_x)
{
    publisher_node_x.clear();
    publisher_node_x = _broadcast_rx_publisher_x;
}
void ConvoyControl::getPublisherNodeY(std::map<int, double> &publisher_node_y)
{
    publisher_node_y.clear();
    publisher_node_y = _broadcast_rx_publisher_y;
}
void ConvoyControl::getPublisherNodeFOV(std::map<int, double> &publisher_node_fov)
{
    publisher_node_fov.clear();
    publisher_node_fov = _broadcast_rx_publisher_fov;
}
void ConvoyControl::getPublisherNodeDirection(std::map<int, int> &publisher_node_direction)
{
    publisher_node_direction.clear();
    publisher_node_direction = _broadcast_rx_publisher_direction;
}
ConvoyControl::AgentStatus ConvoyControl::getAgentStatus()
{
    return (_agent_status);
}

void ConvoyControl::setClusterBroadcastParameters()
{
    unsigned short cluster_id = _member_id;
    if(_cluster_role == Role::MANAGER)
        cluster_id = _manager_id;

    // If the parent module is of station type RSU, include the cluster device corresponding to the parent node in the publisher list
    WrapperNode* wrapper_node = check_and_cast<WrapperNode*>(getParentModule());
    if(wrapper_node->getStationType() == StationType::RSU)
    {
        inet::Coord wrapper_location = wrapper_node->getCurrentLocation();
        double fov = wrapper_node->par("detectorFovLimitRange").doubleValue();
        _broadcast_rx_publisher_id.insert(cluster_id);
        _broadcast_rx_publisher_x.insert(std::make_pair(cluster_id, wrapper_location.x));
        _broadcast_rx_publisher_y.insert(std::make_pair(cluster_id, wrapper_location.y));
        _broadcast_rx_publisher_fov.insert(std::make_pair(cluster_id, fov));
        _broadcast_rx_publisher_direction.insert(std::make_pair(cluster_id, (int) _convoy_direction));
    }

    // Prepare and write cluster broadcast parameters
    std::vector<unsigned short> publisher_id;
    std::vector<double> publisher_x, publisher_y, publisher_fov;
    std::vector<int> publisher_direction;
    for (auto id: _broadcast_rx_publisher_id)
    {
        publisher_id.push_back((unsigned short) id);
        publisher_x.push_back(_broadcast_rx_publisher_x[id]);
        publisher_y.push_back(_broadcast_rx_publisher_y[id]);
        publisher_fov.push_back(_broadcast_rx_publisher_fov[id]);
        publisher_direction.push_back(_broadcast_rx_publisher_direction[id]);
    }

    if(_cluster_role == Role::MANAGER)
    {
        LtePhyEnbD2D *manager_phy = check_and_cast<LtePhyEnbD2D *>(_manager_module->getSubmodule("cellularNic")->getSubmodule("phy"));
        manager_phy->setClusterBroadcastParameters(publisher_id, publisher_x, publisher_y, publisher_fov, publisher_direction);
        manager_phy->setClusterBroadcastParameters(_convoy_id, _manager_id, _convoy_direction, false, false);
    }
    else
    {
        NRPhyUe *member_phy = check_and_cast<NRPhyUe *>(_member_module->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        member_phy->setClusterBroadcastParameters(publisher_id, publisher_x, publisher_y, publisher_fov, publisher_direction);
        member_phy->setClusterBroadcastParameters(_convoy_id, _manager_id, _convoy_direction, false, false);
    }

}

Role ConvoyControl::getClusterRole()
{
    return (_cluster_role);
}

unsigned short ConvoyControl::getManagerId()
{
    return (_manager_id);
}

unsigned short ConvoyControl::getMemberId()
{
    return (_member_id);
}

unsigned short ConvoyControl::getGatewayId()
{
    return (_gateway_id);
}

void ConvoyControl::runModeUpdates()
{
    scheduleAfter(_agent_run_mode_update_interval, _run_mode_update_trigger);
}

void ConvoyControl::receiveCCSMessage(ConvoyControlService *ccs_message)
{
    int max_res_blks = ccs_message->getMax_rb();
    double tx_power = ccs_message->getTx_power();
    double tx_power_gw = ccs_message->getTx_power_gw();
    Role convoy_role = (Role) ccs_message->getConvoy_role();
    Role cluster_role = (Role) ccs_message->getCluster_role();
    int convoy_id = ccs_message->getConvoy_id();
    int cluster_id = ccs_message->getCluster_id();
    int convoy_id_gw = ccs_message->getConvoy_id_gw();
    int cluster_id_gw = ccs_message->getCluster_id_gw();

    // Act further only if the ccs agent is ready
    if(_agent_status == ConvoyControl::AgentStatus::STATUS_INIT_RUN)
    {
        enforceMembership(convoy_role, cluster_role, convoy_id, cluster_id, convoy_id_gw, cluster_id_gw);
        enforceTxPower(cluster_role, tx_power, tx_power_gw);
        if(cluster_role == Role::MANAGER)
            enforceMaxResBlks(max_res_blks);
    }
}

void ConvoyControl::enforceMembership(Role convoy_role, Role cluster_role, int convoy_id, int cluster_id, int convoy_id_gw, int cluster_id_gw)
{
    int old_convoy_id = _convoy_id;
    Role old_convoy_role = _convoy_role;
    int old_cluster_id = _cluster_id;
    Role old_cluster_role = _cluster_role;

    if((convoy_id != old_convoy_id) || (convoy_role != old_convoy_role))
        changeConvoy(convoy_role, convoy_id, convoy_id_gw);
    if((convoy_id != old_convoy_id) || (cluster_id != _cluster_id))
        changeCluster(cluster_role, convoy_id, cluster_id, convoy_id_gw, cluster_id_gw);
    if((convoy_id == old_convoy_id) && (cluster_id == _cluster_id) && (cluster_role != old_cluster_role))
        changeClusterRole(old_cluster_role, cluster_role, convoy_id, cluster_id, convoy_id_gw, cluster_id_gw);
}

void ConvoyControl::changeConvoy(Role convoy_role, int convoy_id, int convoy_id_gw)
{
    if(convoy_role == Role::MANAGER)
        setAsConvoyManager(convoy_id);
    else
    {
        setAsConvoyMember(convoy_id);
        if(convoy_role == Role::GATEWAY)
            setAsConvoyGateway(convoy_id_gw);
    }
}

void ConvoyControl::changeCluster(Role cluster_role, int convoy_id, int cluster_id, int convoy_id_gw, int cluster_id_gw)
{
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    if(cluster_role == Role::MANAGER)
        setAsClusterManager(cluster_id);
    else
    {
        _cluster_id = cluster_id;
        setAsClusterMember(binder->getManagerId(convoy_id, cluster_id), 10.0);
        if(cluster_role == Role::GATEWAY)
        {
            _cluster_id_gw = cluster_id_gw;
            attachClusterGateway(binder->getManagerId(convoy_id_gw, cluster_id_gw), 10.0);
        }
    }
}

void ConvoyControl::changeClusterRole(Role old_cluster_role, Role cluster_role, int convoy_id, int cluster_id, int convoy_id_gw, int cluster_id_gw)
{
    // Manager --> Member (changeCluster)
    // Manager --> Gateway (changeCluster)
    // Member --> Manager (changeCluster)
    // Member --> Gateway (just attach gateway)
    // Gateway --> Manager (change cluster)
    // Gateway --> Member (just detach gateway)
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    if((old_cluster_role == Role::MEMBER) && (cluster_role == Role::GATEWAY))
    {
        cluster_id_gw = cluster_id_gw;
        attachClusterGateway(binder->getManagerId(convoy_id_gw, cluster_id_gw), 10.0);
    }
    else if((old_cluster_role == Role::GATEWAY) && (cluster_role == Role::MEMBER))
        detachClusterGateway();
}

void ConvoyControl::enforceMaxResBlks(int max_res_blks)
{
    //
}
void ConvoyControl::enforceTxPower(Role cluster_role, double tx_power, double tx_power_gw)
{
    if(cluster_role == Role::MANAGER)
    {
        LtePhyEnbD2D *manager_phy = check_and_cast<LtePhyEnbD2D *>(_manager_module->getSubmodule("cellularNic")->getSubmodule("phy"));
        manager_phy->setTxPower(tx_power);
    }
    else
    {
        NRPhyUe *member_phy = check_and_cast<NRPhyUe *>(_member_module->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
        member_phy->setTxPower(tx_power);
        if(cluster_role == Role::GATEWAY)
        {
            NRPhyUe *gateway_phy = check_and_cast<NRPhyUe *>(_gateway_module->getSubmodule("cellularNic")->getSubmodule("nrPhy"));
            gateway_phy->setTxPower(tx_power_gw);
        }
    }
}
void ConvoyControl::sendCCSMessage()
{
    //
}

void ConvoyControl::setAsConvoyGateway(int convoy_id)
{
    // Set up convoy and configure node as convoy member
    _convoy_id_gw = convoy_id;
    _convoy_role = Role::GATEWAY;
}

void ConvoyControl::attachClusterGateway(unsigned short cluster_id_gw, double cluster_rssi_gw)
{
    _cluster_role = Role::GATEWAY;

    // TODO GATEWAY
}

void ConvoyControl::detachClusterGateway()
{
    _cluster_role = Role::MEMBER;
}

double ConvoyControl::getTxp()
{
    // TODO
    return _txp;
}

double ConvoyControl::getTxpGw()
{
    // TODO
    return _txp_gw;
}

ConvoyDirection ConvoyControl::getConvoyDirection()
{
    return _convoy_direction;
}

int ConvoyControl::getConvoyIdCCS()
{
    return _convoy_id_ccs;
}

int ConvoyControl::getClusterIdCCS()
{
    return _cluster_id_ccs;
}
} // namespace convoy_architecture
