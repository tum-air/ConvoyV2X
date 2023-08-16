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

#include "MessagingControl.h"
#include "controls/ConvoyControl.h"
#include "messages/DtwinSub_m.h"
#include "apps/Localizer.h"
#include "inet/common/Units.h"
#include "common/binder/Binder.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "messages/ObjectList_m.h"
#include "stores/DtwinStore.h"
#include "messages/CoopManeuver_m.h"

namespace convoy_architecture {

Define_Module(MessagingControl);

MessagingControl::MessagingControl()
{
    _subscriber_expiry_check_event = nullptr;
}

MessagingControl::~MessagingControl()
{
    cancelAndDelete(_subscriber_expiry_check_event);
}

void MessagingControl::initialize()
{
    _subscriber_expiry_check_event = new omnetpp::cMessage("subscriberExpiryCheckEvent");
    scheduleAfter(par("subscriberExpiryCheckInterval").doubleValue(), _subscriber_expiry_check_event);
}

void MessagingControl::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    if(!msg->isSelfMessage())
    {
        if(msg->arrivedOn("inUlAppDtwinSub"))
        {
            EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "Received subscription request message from upper layer" << std::endl;
            if(getParentModule()->hasSubmodule("ccsAgent"))
                handleDtwinSubscriptionMsgFromUl(msg);
            else
                EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "ccs agent not yet ready, ignoring message " << std::endl;
            delete msg;
        }
        else if(msg->arrivedOn("inUlAppDtwinPub"))
        {
            EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "Received dtwin message from upper layer" << std::endl;
            if(getParentModule()->hasSubmodule("ccsAgent"))
                handleDtwinPublicationMsgFromUl(msg);
            else
                EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "ccs agent not yet ready, ignoring message " << std::endl;
            delete msg;
        }
        else if(msg->arrivedOn("inUlAppCoopMan"))
        {
            EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "Received coop. man. message from upper layer" << std::endl;
            if(getParentModule()->hasSubmodule("ccsAgent"))
                handleCoopManMsgFromUl(msg);
            else
                EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "ccs agent not yet ready, ignoring message " << std::endl;
            delete msg;
        }
        else if (msg->arrivedOn("inLlAppDtwinSub"))
        {
            EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "Received dtwin message from lower layer" << std::endl;
            if(getParentModule()->hasSubmodule("ccsAgent"))
                handleDtwinPublicationMsgFromLl(msg);
            else
            {
                EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "ccs agent not yet ready, ignoring message " << std::endl;
                delete msg;
            }
        }
        else if (msg->arrivedOn("inLlAppDtwinPub"))
        {
            EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "Received subscription request message from lower layer" << std::endl;
            if(getParentModule()->hasSubmodule("ccsAgent"))
                handleDtwinSubscriptionMsgFromLl(msg);
            else
            {
                EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "ccs agent not yet ready, ignoring message " << std::endl;
                delete msg;
            }
        }
        else if(msg->arrivedOn("inLlAppCoopMan"))
        {
            EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "Received coop. man. message from lower layer" << std::endl;
            if(getParentModule()->hasSubmodule("ccsAgent"))
                handleCoopManMsgFromLl(msg);
            else
            {
                EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "ccs agent not yet ready, ignoring message " << std::endl;
                delete msg;
            }
        }

    }
    else if (msg == _subscriber_expiry_check_event)
    {
        EV_INFO << current_time <<" - MessagingControl::handleMessage(): " << "Subscriber-expiry check triggered " << std::endl;
        trimSubscriberList();
        scheduleAfter(par("subscriberExpiryCheckInterval").doubleValue(), _subscriber_expiry_check_event);
    }
}

void MessagingControl::handleDtwinSubscriptionMsgFromUl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    DtwinSub *dtwin_sub_msg = check_and_cast<DtwinSub *>(msg);
    ConvoyControl *ccs_agent_module = check_and_cast<ConvoyControl *>(getParentModule()->getSubmodule("ccsAgent"));
    Localizer *app_localizer = check_and_cast<Localizer *>(getParentModule()->getSubmodule("appLocalizer"));

    // Proceed further only if ccs agent initialization with broadcast scanning is done
    if(ccs_agent_module->getAgentStatus() != ConvoyControl::AgentStatus::STATUS_INIT_SCANNING)
    {
        // Get the dtwin publisher infos
        std::set<int> publisher_node_id;
        std::map<int, double> publisher_node_x, publisher_node_fov;
        std::map<int, int> publisher_node_direction;
        ccs_agent_module->getPublisherNodeID(publisher_node_id);
        ccs_agent_module->getPublisherNodeX(publisher_node_x);
        ccs_agent_module->getPublisherNodeFOV(publisher_node_fov);
        ccs_agent_module->getPublisherNodeDirection(publisher_node_direction);

        // Decide which publishers are to receive the subscription request
        // Check for direction match and fov match
        // If the node has not completed initial localization
        // send request to all publishers in the same direction and which are in the same cluster
        std::set<int> target_publisher_node_id;
        for(auto node_id:publisher_node_id)
        {
            if(publisher_node_direction[node_id] == par("convoyDirection").intValue())
            {
                EV_INFO << current_time <<" - MessagingControl::handleDtwinSubscriptionMsgFromUl(): Publisher " << node_id << " found in same direction" << std::endl;
                bool fov_match = true;
                if(app_localizer->isLocalized())
                {
                    EV_INFO << current_time <<" - MessagingControl::handleDtwinSubscriptionMsgFromUl(): Localization completed, checking for fov match" << std::endl;
                    double ego_x = app_localizer->readEgoPositionX();
                    std::string roi_tag = std::string(dtwin_sub_msg->getRoi_tag());
                    double rsu_x = publisher_node_x[node_id];
                    double rsu_fov = publisher_node_fov[node_id];
                    fov_match = fovMatch(ego_x, roi_tag, rsu_x, rsu_fov);
                }
                else
                {
                    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
                    if(ccs_agent_module->getManagerId() != binder->getNextHop((MacNodeId) node_id))
                        fov_match = false;
                    EV_INFO << current_time <<" - MessagingControl::handleDtwinSubscriptionMsgFromUl(): Localization not yet completed" << std::endl;
                }
                if(fov_match)
                {
                    EV_INFO << current_time <<" - MessagingControl::handleDtwinSubscriptionMsgFromUl(): Adding publisher to target list" << std::endl;
                    target_publisher_node_id.insert(node_id);
                }
                else
                    EV_INFO << current_time <<" - MessagingControl::handleDtwinSubscriptionMsgFromUl(): Publisher fov does not match, ignoring" << std::endl;
            }
            else
                EV_INFO << current_time <<" - MessagingControl::handleDtwinSubscriptionMsgFromUl(): Publisher " << node_id << " not in same direction, ignoring" << std::endl;
        }

        // Forward request packets to cluster device
        for (auto destination_id: target_publisher_node_id)
            routeMessageFromUlToDestination(msg, destination_id, MessagingControl::ApplicationType::DTWIN_SUB);
    }
    else
        EV_INFO << current_time <<" - MessagingControl::handleDtwinSubscriptionMsgFromUl(): Convoy control agent not done scanning for cluster broadcast messages, ignoring subscription request" << std::endl;
}

bool MessagingControl::fovMatch(double ego_x, std::string roi_tag, double rsu_x, double rsu_fov)
{
    double ego_fov = par("roiNearby").doubleValue();
    if (roi_tag == std::string("horizon"))
        ego_fov = par("roiHorizon").doubleValue();

    double border_ego_min = ego_x - ego_fov;
    double border_ego_max = ego_x + ego_fov;
    double border_rsu_min = rsu_x - rsu_fov;
    double border_rsu_max = rsu_x + rsu_fov;

    bool ego_min_in_fov = (border_ego_min >= border_rsu_min) && (border_ego_min <= border_rsu_max);
    bool ego_max_in_fov = (border_ego_max >= border_rsu_min) && (border_ego_max <= border_rsu_max);
    return(ego_min_in_fov || ego_max_in_fov);
}

void MessagingControl::handleDtwinSubscriptionMsgFromLl(omnetpp::cMessage *msg)
{
    routeMessageFromLlToDestination(msg, MessagingControl::ApplicationType::DTWIN_SUB);
}

void MessagingControl::routeMessageFromLlToDestination(omnetpp::cMessage *msg, MessagingControl::ApplicationType app_type)
{
    // Check for routing first
    // If the message destination address matches this device, forward the message to the application layer
    // If this device is a cluster member and not a gateway - drop the message
    // If this device is a cluster member and a gateway, and also has a route to the destination cluster, forward the message to the gateway device
    // If not drop the message
    // If this device is a cluster manager and if the destination cluster matches, forward the message to the final destination in the same cluster
    // If this device is a cluster manager and if there is route to the destination cluster, forward the message to the member in the same cluster which has a gateway with a route to the destination cluster
    // If this device is a cluster manager and if there is no route to the destination cluster, drop the message

    omnetpp::simtime_t current_time = omnetpp::simTime();
    ConvoyControl *ccs_agent_module = check_and_cast<ConvoyControl *>(getParentModule()->getSubmodule("ccsAgent"));
    inet::Packet *packet = check_and_cast<inet::Packet *>(msg);
    auto mcs_packet = packet->popAtFront<MCSPacket>();
    int msg_hop_cluster = mcs_packet->getHop_cluster();
    int msg_dst_mac_id = mcs_packet->getDst_mac_id();
    int self_cluster = (int) ccs_agent_module->getManagerId();
    int self_mac_id = (ccs_agent_module->getClusterRole() == ConvoyControl::Role::MANAGER)? ((int) ccs_agent_module->getManagerId()) : ((int) ccs_agent_module->getMemberId());

    if(msg_dst_mac_id == self_mac_id)
    {
        EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): destination id matches this device, forwarding packet to application layer" << std::endl;
        if(app_type == MessagingControl::ApplicationType::DTWIN_SUB)
        {
            // Add to subscriber list
            std::string subscriber_id = std::string(mcs_packet->getMsg_dtwin_sub().getSubscriber_id());
            _subscriber_id.insert(subscriber_id);
            _subscriber_address[subscriber_id] = mcs_packet->getSrc_mac_id();
            _subscriber_timestamp[subscriber_id] = mcs_packet->getMsg_dtwin_sub().getTimestamp();
            _subscriber_roi[subscriber_id] = std::string(mcs_packet->getMsg_dtwin_sub().getRoi_tag());
            _subscriber_qos[subscriber_id] = std::string(mcs_packet->getMsg_dtwin_sub().getQos_tag());

            // Update subscriber type and address in dtwin store if possible
            if(getParentModule()->hasSubmodule("dtwinStore"))
            {
                DtwinStore *dtwin_store = check_and_cast<DtwinStore*>(getParentModule()->getSubmodule("dtwinStore"));
                dtwin_store->updateObjectTypeAndAddress(subscriber_id, std::string(mcs_packet->getMsg_dtwin_sub().getSubscriber_type()), mcs_packet->getMsg_dtwin_sub().getSubscriber_address());
            }
            EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): subscriber " << subscriber_id << " added to list, list length = " << _subscriber_timestamp.size() << std::endl;
        }
        else if(app_type == MessagingControl::ApplicationType::DTWIN_PUB)
        {
            // Forward dtwin message to application layer
            ObjectList *msg_dtwin_pub = mcs_packet->getMsg_dtwin_pub().dup();
            send(msg_dtwin_pub, "outUlAppDtwinSub");
        }
        else if(app_type == MessagingControl::ApplicationType::COOP_MANEUVER)
        {
            // Forward coop. man. message to application layer
            CoopManeuver *msg_coop_man = mcs_packet->getMsg_coop_man().dup();
            send(msg_coop_man, "outUlAppCoopMan");
        }
    }
    else if(ccs_agent_module->getClusterRole() == ConvoyControl::Role::MEMBER)
        EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): destination id does not match this member node, dropping message" << std::endl;
    else if(ccs_agent_module->getClusterRole() == ConvoyControl::Role::GATEWAY)
    {
        // TODO: Message to be forwarded to gateway if a route to destination cluster is available, else to be dropped.
        EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): destination id does not match this gateway node, dropping message for now" << std::endl;
    }
    else if(ccs_agent_module->getClusterRole() == ConvoyControl::Role::MANAGER)
    {
        if (self_cluster == mcs_packet->getDst_cluster())
        {
            // Set the next hop parameters
            // Construct a new packet to avoid residual chunks from original packet
            inet::Packet *modified_packet = new inet::Packet(packet->getName());
            auto modified_mcs_packet = inet::makeShared<MCSPacket>(*mcs_packet->dup());
            modified_mcs_packet->setHop_cluster(self_cluster);
            modified_mcs_packet->setHop_mac_id(msg_dst_mac_id);
            modified_packet->insertAtBack(modified_mcs_packet);

            if(app_type == MessagingControl::ApplicationType::DTWIN_SUB)
            {
                std::string destination_module_name = setDestinationAddress(modified_packet, msg_dst_mac_id, par("destPortAppDtwinSub").intValue());

                EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): sending dtwin subscription packet to " << destination_module_name << std::endl;
                send(modified_packet, "outLlAppDtwinSub");
            }
            else if(app_type == MessagingControl::ApplicationType::DTWIN_PUB)
            {
                std::string destination_module_name = setDestinationAddress(modified_packet, msg_dst_mac_id, par("destPortAppDtwinPub").intValue());

                EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): sending dtwin message to " << destination_module_name << std::endl;
                send(modified_packet, "outLlAppDtwinPub");
            }
            else if(app_type == MessagingControl::ApplicationType::COOP_MANEUVER)
            {
                std::string destination_module_name = setDestinationAddress(modified_packet, msg_dst_mac_id, par("destPortAppCoopMan").intValue());

                EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): sending coop. man. message to " << destination_module_name << std::endl;
                send(modified_packet, "outLlAppCoopMan");
            }
        }
        else
        {
            // TODO: else if there is a route to destination cluster, forward the message to the member in the same cluster which has a gateway with a route to the destination cluster
            // TODO: else drop the message since no route is available
            EV_INFO << current_time <<" - MessagingControl::routeMessageFromLlToDestination(): destination id does not match this manager node, dropping message for now" << std::endl;
        }
    }
    delete msg;
}

void MessagingControl::routeMessageFromUlToDestination(omnetpp::cMessage *msg, int destination_id, MessagingControl::ApplicationType app_type)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    ConvoyControl *ccs_agent_module = check_and_cast<ConvoyControl *>(getParentModule()->getSubmodule("ccsAgent"));
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));

    // Construct and fill up packet for transfer
    auto mcs_packet = inet::makeShared<MCSPacket>();
    std::string packet_name = "DtwinSubscription";
    std::string dest_port = "destPortAppDtwinSub";
    std::string out_gate = "outLlAppDtwinSub";

    // Setup source and destination ids
    mcs_packet->setSrc_cluster((int) ccs_agent_module->getManagerId());
    int src_mac_id = (ccs_agent_module->getClusterRole() == ConvoyControl::Role::MANAGER)? ((int) ccs_agent_module->getManagerId()) : ((int) ccs_agent_module->getMemberId());
    mcs_packet->setSrc_mac_id(src_mac_id);
    mcs_packet->setDst_cluster(binder->getNextHop((MacNodeId) destination_id)); // Can also be added to publisher broadcast
    mcs_packet->setDst_mac_id(destination_id);

    if(app_type == MessagingControl::ApplicationType::DTWIN_SUB)
    {
        DtwinSub *dtwin_sub_msg = check_and_cast<DtwinSub *>(msg);
        dtwin_sub_msg->setSubscriber_address(src_mac_id);
        mcs_packet->setTimestamp(dtwin_sub_msg->getTimestamp());
        mcs_packet->setMsg_dtwin_sub(*dtwin_sub_msg);
        mcs_packet->setChunkLength(inet::B(par("sizeDtwinSubMsg").intValue()));
    }
    else if(app_type == MessagingControl::ApplicationType::DTWIN_PUB)
    {
        packet_name = "DtwinPublication";
        dest_port = "destPortAppDtwinPub";
        out_gate = "outLlAppDtwinPub";
        ObjectList *dtwin_pub_msg = check_and_cast<ObjectList *>(msg);
        mcs_packet->setTimestamp(dtwin_pub_msg->getTimestamp());
        mcs_packet->setChunkLength(inet::B(dtwin_pub_msg->getObj_byte_size() * dtwin_pub_msg->getN_objects()));
        mcs_packet->setMsg_dtwin_pub(*dtwin_pub_msg);
    }
    else if(app_type == MessagingControl::ApplicationType::COOP_MANEUVER)
    {
        packet_name = "CooperativeManeuver";
        dest_port = "destPortAppCoopMan";
        out_gate = "outLlAppCoopMan";
        CoopManeuver *coop_man_msg = check_and_cast<CoopManeuver *>(msg);
        mcs_packet->setTimestamp(coop_man_msg->getTimestamp());
        mcs_packet->setChunkLength(inet::B(par("sizeCoopManMsg").intValue()));
        mcs_packet->setMsg_coop_man(*coop_man_msg);
    }
    else
        return;

    // Check for routing first
    // If this node is a cluster member - next hop is its cluster manager, forward message to member device
    // If this node is a cluster gateway, but destination cluster matches member device cluster, do the same as above
    // If this node is a cluster gateway, and if the gateway device has a route to destination cluster, next hop is the gateway's cluster manager, forward message to gateway device
    // If this node is a cluster manager and if the destination cluster matches, next hop is the final destination, forward the message to manager device
    // If this node is a cluster manager and if there is a route to the destination cluster, next hop is the member in the same cluster which has a gateway with a route to the destination cluster, forward the message to manager device
    // If this node is a cluster manager and if there is no route to the destination cluster, drop the message

    int hop_cluster = 0;
    int hop_mac_id = 0;
    switch (ccs_agent_module->getClusterRole())
    {
    case ConvoyControl::Role::MEMBER:
        hop_cluster = ccs_agent_module->getManagerId();
        hop_mac_id = ccs_agent_module->getManagerId();
        break;
    case ConvoyControl::Role::GATEWAY:
        if(mcs_packet->getDst_cluster() == ccs_agent_module->getManagerId())
        {
            hop_cluster = ccs_agent_module->getManagerId();
            hop_mac_id = ccs_agent_module->getManagerId();
        }
        else
        {
            // TODO: Gateway function
            out_gate = "outLlGwAppDtwinSub";
            return;
        }
        break;
    default:
        if(mcs_packet->getDst_cluster() == ccs_agent_module->getManagerId())
        {
            hop_cluster = ccs_agent_module->getManagerId();
            hop_mac_id = mcs_packet->getDst_mac_id();
        }
        else
        {
            // TODO: Gateway route availability check
            return;
        }
        break;
    }
    EV_INFO << current_time <<" - MessagingControl::routeMessageFromUlToDestination(): next hop cluster = " << hop_cluster << ", next hop id = " << hop_mac_id << std::endl;
    EV_INFO << current_time <<" - MessagingControl::routeMessageFromUlToDestination(): final dst cluster = " << mcs_packet->getDst_cluster() << ", final dst id = " << mcs_packet->getDst_mac_id() << std::endl;

    mcs_packet->setHop_cluster(hop_cluster);
    mcs_packet->setHop_mac_id(hop_mac_id);
    inet::Packet *packet = new inet::Packet(packet_name.c_str());

    // Insert message into transfer packet
    packet->insertAtBack(mcs_packet);

    // Set packet destination for next hop address
    std::string destination_module_name = setDestinationAddress(packet, hop_mac_id, par(dest_port.c_str()).intValue());

    EV_INFO << current_time <<" - MessagingControl::routeMessageFromUlToDestination(): sending mcs packet to next hop: " << destination_module_name << std::endl;
    send(packet, out_gate.c_str());
}

std::string MessagingControl::setDestinationAddress(inet:: Packet *packet, int hop_mac_id, int application_port)
{
    // Set packet destination for next hop address
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    omnetpp::cModule *destination_module = getSimulation()->getModule(binder->getOmnetId((MacNodeId) hop_mac_id));
    inet::L3Address destination_address = inet::L3AddressResolver().resolve(destination_module->getFullName());
    auto addressReq = packet->addTagIfAbsent<inet::L3AddressReq>();
    addressReq->setDestAddress(destination_address);
    auto portReq = packet->addTagIfAbsent<inet::L4PortReq>();
    portReq->setDestPort(application_port);

    std::string destination_module_name = destination_module->getFullName();
    return (destination_module_name);
}

void MessagingControl::handleDtwinPublicationMsgFromLl(omnetpp::cMessage *msg)
{
    routeMessageFromLlToDestination(msg, MessagingControl::ApplicationType::DTWIN_PUB);
}

void MessagingControl::handleDtwinPublicationMsgFromUl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    ObjectList *dtwin_pub_msg = check_and_cast<ObjectList *>(msg);
    ConvoyControl *ccs_agent_module = check_and_cast<ConvoyControl *>(getParentModule()->getSubmodule("ccsAgent"));

    // Proceed further only if ccs agent initialization with broadcast scanning is done
    if(ccs_agent_module->getAgentStatus() != ConvoyControl::AgentStatus::STATUS_INIT_SCANNING)
    {
        // Forward dtwin message to subscribers if present
        if(_subscriber_id.size() > 0)
        {
            for (auto subscriber_id : _subscriber_id)
                routeMessageFromUlToDestination(msg, _subscriber_address[subscriber_id], MessagingControl::ApplicationType::DTWIN_PUB);
        }
        else
            EV_INFO << current_time <<" - MessagingControl::handleDtwinPublicationMsgFromUl(): no known subscribers available, ignoring dtwin message" << std::endl;
    }
    else
        EV_INFO << current_time <<" - MessagingControl::handleDtwinPublicationMsgFromUl(): Convoy control agent not done scanning for cluster broadcast messages, ignoring dtwin message" << std::endl;
}

void MessagingControl::trimSubscriberList()
{
    const omnetpp::simtime_t current_time = omnetpp::simTime();
    const double max_age = par("subscriberMaxAge").doubleValue();

    // Iterate through the subscriber list and remove expired entries
    for (auto it = _subscriber_timestamp.cbegin(); it != _subscriber_timestamp.cend();)
    {
        const int64_t subscriber_timestamp = (int64_t) it->second;
        const omnetpp::simtime_t subscriber_time = omnetpp::simtime_t::fromRaw(subscriber_timestamp);
        if((current_time - subscriber_time).dbl() >= max_age)
        {
            EV_INFO << current_time <<" - MessagingControl::trimSubscriberList(): max age threshold reached for subscriber " << it->first << ", removing from list" << std::endl;
            _subscriber_id.erase(it->first);
            _subscriber_address.erase(it->first);
            _subscriber_roi.erase(it->first);
            _subscriber_qos.erase(it->first);
            it = _subscriber_timestamp.erase(it);
        }
        else
            ++it;
    }
}

void MessagingControl::handleCoopManMsgFromLl(omnetpp::cMessage *msg)
{
    routeMessageFromLlToDestination(msg, MessagingControl::ApplicationType::COOP_MANEUVER);
}

void MessagingControl::handleCoopManMsgFromUl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    CoopManeuver *coop_man_msg = check_and_cast<CoopManeuver *>(msg);
    ConvoyControl *ccs_agent_module = check_and_cast<ConvoyControl *>(getParentModule()->getSubmodule("ccsAgent"));

    // Forward cooperative maneuvering message to partner only if ccs agent initialization with broadcast scanning is done
    if(ccs_agent_module->getAgentStatus() != ConvoyControl::AgentStatus::STATUS_INIT_SCANNING)
        routeMessageFromUlToDestination(msg, coop_man_msg->getPartner_address(), MessagingControl::ApplicationType::COOP_MANEUVER);
    else
        EV_INFO << current_time <<" - MessagingControl::handleCoopManMsgFromUl(): Convoy control agent not done scanning for cluster broadcast messages, ignoring dtwin message" << std::endl;
}
} // namespace convoy_architecture
