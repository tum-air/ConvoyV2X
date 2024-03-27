
#include "RoutingControl.h"
#include "messages/DtwinSub_m.h"
#include "messages/ObjectList_m.h"
#include "messages/CoopManeuver_m.h"
#include "messages/MemberStatus_m.h"
#include "messages/Orchestration_m.h"
#include "controls/MembershipControl.h"
#include "apps/Localizer.h"
#include "stores/SubscriberStore.h"
#include "common/binder/Binder.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace convoy_architecture {

Define_Module(RoutingControl);

void RoutingControl::initialize() {
    // TODO - Generated method body
}

void RoutingControl::handleMessage(omnetpp::cMessage *msg) {
    if(msg->arrivedOn("inUlSubscriber") || msg->arrivedOn("inLlSubscriber") || msg->arrivedOn("inLlSubscriberGw"))
        msgHandlerSubscriber(msg);

    else if(msg->arrivedOn("inUlPublisher") || msg->arrivedOn("inLlPublisher") || msg->arrivedOn("inLlPublisherGw"))
        msghandlerPublisher(msg);

    else if(msg->arrivedOn("inUlManeuver") || msg->arrivedOn("inLlManeuver") || msg->arrivedOn("inLlManeuverGw"))
        msgHandlerManeuver(msg);

    else if(msg->arrivedOn("inUlMemberReport") || msg->arrivedOn("inLlMemberReport") || msg->arrivedOn("inLlMemberReportGw"))
        msgHandlerMemberReport(msg);

    else if(msg->arrivedOn("inUlMemberControl") || msg->arrivedOn("inLlMemberControl") || msg->arrivedOn("inLlMemberControlGw") || msg->arrivedOn("inBkndMemberControl"))
        msgHandlerOrchestration(msg);
}

void RoutingControl::msgHandlerSubscriber(omnetpp::cMessage *msg) {

    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerSubscriber(): " << "Received message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlSubscriber") {
        if(getParentModule()->hasSubmoduleVector("membershipControl")) {
            MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
            if(membership_control->isInitialized()) {
                ConvoyDirection direction = (ConvoyDirection) par("convoyDirection").intValue();

                // Get the dtwin publisher infos
                const std::vector<Publication>& publishers = membership_control->readPublishers();

                // Decide which publishers are to receive the subscription request
                // First extract those publishers which cover the same direction
                std::vector<Publication> publishers_same_direction;
                std::copy_if(std::begin(publishers), std::end(publishers), std::back_inserter(publishers_same_direction), [direction] (Publication const& p) {
                    return (p.direction == direction);
                });

                // If the node has not completed initial localization
                Localizer *app_localizer = check_and_cast<Localizer *>(getParentModule()->getSubmodule("appLocalizer"));
                std::vector<Publication> publishers_filtered;
                if(!app_localizer->isLocalized()) {
                    // send request to all publishers in the same direction and which are in the same cluster
                    int cluster = membership_control->getManagerID();
                    std::copy_if(std::begin(publishers_same_direction), std::end(publishers_same_direction), std::back_inserter(publishers_filtered), [cluster] (Publication const& p) {
                        return (p.cluster == cluster);
                    });
                }
                else {
                    // else send request to those publishers with matching fov
                    double ego_x = app_localizer->readEgoPositionX();
                    DtwinSub *msg_subscription = check_and_cast<DtwinSub *>(msg);
                    std::string roi = std::string{msg_subscription->getRoi_tag()};
                    double ego_fov = par("roiNearby").doubleValue();
                    if (roi == std::string("horizon"))
                        ego_fov = par("roiHorizon").doubleValue();
                    std::copy_if(std::begin(publishers_same_direction), std::end(publishers_same_direction), std::back_inserter(publishers_filtered), [ego_x, ego_fov] (Publication const& p) {
                        double border_ego_min = ego_x - ego_fov;
                        double border_ego_max = ego_x + ego_fov;
                        double border_rsu_min = p.position.x - p.fov;
                        double border_rsu_max = p.position.x + p.fov;
                        bool ego_min_in_fov = (border_ego_min >= border_rsu_min) && (border_ego_min <= border_rsu_max);
                        bool ego_max_in_fov = (border_ego_max >= border_rsu_min) && (border_ego_max <= border_rsu_max);
                        return(ego_min_in_fov || ego_max_in_fov);
                    });
                }

                // Send out the subscription requests
                std::for_each(std::begin(publishers_filtered), std::end(publishers_filtered), [this, msg] (Publication const& p) {
                    TransportPacket *transport_packet = new TransportPacket();
                    DtwinSub *dtwin_sub_msg = check_and_cast<DtwinSub *>(msg->dup());
                    transport_packet->setTimestamp(dtwin_sub_msg->getTimestamp());
                    transport_packet->setMsg_subscriber(*dtwin_sub_msg);
                    transport_packet->setChunkLength(inet::B(par("sizeSubscriberMsg").intValue()));
                    transport_packet->setDst_mac_id(p.id);
                    forwardToNetwork(transport_packet, MessageType::SUBSCRIPTION);
                });
                delete msg;
            }
            else
                delete msg;
        }
        else
            delete msg;
    }
    else
        receiveFromNetwork(check_and_cast<inet::Packet *>(msg), MessageType::SUBSCRIPTION);
}

void RoutingControl::msghandlerPublisher(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msghandlerPublisher(): " << "Received message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlPublisher") {
        if(getParentModule()->hasSubmoduleVector("membershipControl")) {
            MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
            if(membership_control->isInitialized()) {
                SubscriberStore *store = check_and_cast<SubscriberStore *>(getParentModule()->getSubmodule("subscriberStore"));
                const std::vector<Subscription>& subscribers = store->readSubscriptions();
                if(subscribers.size() > 0)
                {
                    // Send out the dtwin publications to the subcribers
                    std::for_each(std::begin(subscribers), std::end(subscribers), [this, msg] (Subscription const& s) {
                        TransportPacket *transport_packet = new TransportPacket();
                        ObjectList *dtwin_pub_msg = check_and_cast<ObjectList *>(msg->dup());
                        transport_packet->setTimestamp(dtwin_pub_msg->getTimestamp());
                        transport_packet->setChunkLength(inet::B(dtwin_pub_msg->getObj_byte_size() * dtwin_pub_msg->getN_objects()));
                        transport_packet->setMsg_publisher(*dtwin_pub_msg);
                        transport_packet->setDst_mac_id(s.id);
                        forwardToNetwork(transport_packet, MessageType::PUBLICATION);
                    });

                    // Send out a copy to the backend
                    if(par("stationType").intValue() == StationType::RSU) {
                        TransportPacket *transport_packet = new TransportPacket();
                        ObjectList *msg_dtwin = check_and_cast<ObjectList *>(msg);
                        transport_packet->setTimestamp(msg_dtwin->getTimestamp());
                        transport_packet->setChunkLength(inet::B(msg_dtwin->getObj_byte_size() * msg_dtwin->getN_objects()));
                        transport_packet->setMsg_publisher(*msg_dtwin);
                        forwardToBackend(transport_packet, MessageType::PUBLICATION);
                    }
                    else
                        delete msg;
                }
                else {
                    EV_INFO << current_time <<" - RoutingControl::msghandlerPublisher: no known subscribers available, ignoring message" << std::endl;
                    delete msg;
                }
            }
            else
                delete msg;
        }
        else
            delete msg;
    }
    else
        receiveFromNetwork(check_and_cast<inet::Packet *>(msg), MessageType::PUBLICATION);
}

void RoutingControl::msgHandlerManeuver(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerManeuver(): " << "Received message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlManeuver") {
        if(getParentModule()->hasSubmoduleVector("membershipControl")) {
            MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
            if(membership_control->isInitialized()) {
                TransportPacket *transport_packet = new TransportPacket();
                CoopManeuver *coop_man_msg = check_and_cast<CoopManeuver *>(msg);
                transport_packet->setTimestamp(coop_man_msg->getTimestamp());
                transport_packet->setChunkLength(inet::B(par("sizeManeuverMsg").intValue()));
                transport_packet->setMsg_maneuver(*coop_man_msg);
                transport_packet->setDst_mac_id(coop_man_msg->getPartner_address());
                forwardToNetwork(transport_packet, MessageType::MANEUVER);
            }
            else
                delete msg;
        }
        else
            delete msg;
    }
    else
        receiveFromNetwork(check_and_cast<inet::Packet *>(msg), MessageType::MANEUVER);
}

void RoutingControl::msgHandlerMemberReport(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerMemberReport(): " << "Received member status message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlMemberReport") {
        // Member status report from the upper layer is sent to backend interface if the node is a rsu
        if(par("stationType").intValue() == StationType::RSU) {
            TransportPacket *transport_packet = new TransportPacket();
            MemberStatus *msg_member_status = check_and_cast<MemberStatus *>(msg);
            transport_packet->setTimestamp(msg_member_status->getTimestamp());
            transport_packet->setChunkLength(inet::B(par("sizeMemberReportMsg").intValue()));
            transport_packet->setMsg_member_status(*msg_member_status);
            forwardToBackend(transport_packet, MessageType::MEMBER_STATUS);
        }
        // Else it is sent to the publishers in the same cluster
        // If there are no publishers in the same cluster, it is sent to all publishers serving the same convoy direction
        else {
            MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
            ConvoyDirection direction = (ConvoyDirection) par("convoyDirection").intValue();
            int cluster = membership_control->getManagerID();
            const std::vector<Publication>& publishers = membership_control->readPublishers();
            std::vector<Publication> publishers_same_direction;
            std::copy_if(std::begin(publishers), std::end(publishers), std::back_inserter(publishers_same_direction), [direction] (Publication const& p) {
                return (p.direction == direction);
            });
            auto end = std::partition(std::begin(publishers_same_direction), std::end(publishers_same_direction), [cluster] (Publication const& p) {
                return (p.cluster == cluster);
            });
            auto target_publishers_begin = std::begin(publishers_same_direction);
            auto target_publishers_end = (end == std::begin(publishers_same_direction))? std::end(publishers_same_direction) : end;
            // Send out the member reports
            std::for_each(target_publishers_begin, target_publishers_end, [this, msg] (Publication const& p) {
                TransportPacket *transport_packet = new TransportPacket();
                MemberStatus *msg_member_status = check_and_cast<MemberStatus *>(msg->dup());
                transport_packet->setTimestamp(msg_member_status->getTimestamp());
                transport_packet->setChunkLength(inet::B(par("sizeMemberReportMsg").intValue()));
                transport_packet->setMsg_member_status(*msg_member_status);
                transport_packet->setDst_mac_id(p.id);
                forwardToNetwork(transport_packet, MessageType::MEMBER_STATUS);
            });
            delete msg;
        }
    }
    else {
        // Member status report from the lower layer is sent to backend interface if the node is a rsu
        if(par("stationType").intValue() == StationType::RSU) {
            // Extract member status transport packet
            inet::Packet *packet = omnetpp::check_and_cast<inet::Packet *>(msg);
            auto transport_packet = packet->popAtFront<TransportPacket>();
            forwardToBackend(transport_packet->dup(), MessageType::MEMBER_STATUS);
            delete msg;
        }
        // Else send it to next hop
        else
            receiveFromNetwork(check_and_cast<inet::Packet *>(msg), MessageType::MEMBER_STATUS);
    }
}

void RoutingControl::msgHandlerOrchestration(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerOrchestration(): " << "Received orchestration message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlMemberControl") {
        // Message received from upper layer
        TransportPacket *transport_packet = new TransportPacket();
        Orchestration * msg_orchestration = check_and_cast<Orchestration *>(msg);
        transport_packet->setTimestamp(msg_orchestration->getTimestamp());
        transport_packet->setChunkLength(inet::B(par("sizeOrchestrationMsg").intValue()));
        transport_packet->setMsg_orchestration(*msg_orchestration);
        transport_packet->setDst_mac_id(msg_orchestration->getNode_address());
        forwardToNetwork(transport_packet, MessageType::ORCHESTRATION);
    }
    else if(arrival_gate == "inBkndMemberControl") {
        MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
        TransportPacket *transport_packet = check_and_cast<TransportPacket *>(msg);
        if(membership_control->addressMatch(transport_packet->getDst_mac_id())) {
            Orchestration *msg_orchestration = transport_packet->getMsg_orchestration().dup();
            send(msg_orchestration, "outUlMemberControl");
            delete msg;
        }
        else
            forwardToNetwork(transport_packet, MessageType::ORCHESTRATION);
    }
    else {
        // Message received from lower layer
        receiveFromNetwork(check_and_cast<inet::Packet *>(msg), MessageType::ORCHESTRATION);
    }
}

void RoutingControl::receiveFromNetwork(inet::Packet *network_packet, MessageType type) {
    // 1. Check if node is of type backend
    // 2. Forward packet to upper layer if this is the case
    // 3. Else check if packet is meant for this node
    // 4. Forward to upper layer if this is the case
    // 5. Else forward packet back to network for multi-hop routing

    auto transport_packet = network_packet->popAtFront<TransportPacket>();
    MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
    if(par("stationType").intValue() == StationType::BACKEND) {
        if(type == MessageType::PUBLICATION) {
            ObjectList *msg_dtwin = transport_packet->getMsg_publisher().dup();
            send(msg_dtwin, "outUlSubscriber");
        }
        else if(type == MessageType::MEMBER_STATUS) {
            MemberStatus *msg_member_status = transport_packet->getMsg_member_status().dup();
            send(msg_member_status, "outUlMemberControl");
        }
    }
    else if(membership_control->addressMatch(transport_packet->getDst_mac_id())) {
        if(type == MessageType::SUBSCRIPTION) {
            DtwinSub *msg_subscription = transport_packet->getMsg_subscriber().dup();
            send(msg_subscription, "outUlPublisher");
        }
        else if(type == MessageType::PUBLICATION) {
            ObjectList *msg_publisher = transport_packet->getMsg_publisher().dup();
            send(msg_publisher, "outUlSubscriber");
        }
        else if(type == MessageType::MANEUVER) {
            CoopManeuver *msg_maneuver = transport_packet->getMsg_maneuver().dup();
            send(msg_maneuver, "outUlManeuver");
        }
        else if(type == MessageType::MEMBER_STATUS) {
            MemberStatus *msg_member_status = transport_packet->getMsg_member_status().dup();
            send(msg_member_status, "outUlMemberControl");
        }
        else if(type == MessageType::ORCHESTRATION) {
            Orchestration *msg_orchestration = transport_packet->getMsg_orchestration().dup();
            send(msg_orchestration, "outUlMemberControl");
        }
    }
    else {
        forwardToNetwork(transport_packet->dup(), type);
    }
    delete network_packet;
}

void RoutingControl::forwardToNetwork(TransportPacket *transport_packet, MessageType type) {
    // 1. Find next hop for the transport packet
    // 2. Create new inet packet and transfer to corresponding NIC

    std::string packet_name{""}, dest_port{""}, out_gate{""};
    int chunk_length{0}, hop_mac_id{0};
    if(type == MessageType::SUBSCRIPTION) {
        packet_name = "Subscription";
        dest_port = "destPortSubscriber";
        out_gate = "outLlSubscriber";
        chunk_length = par("sizeSubscriberMsg").intValue();
    }
    else if(type == MessageType::PUBLICATION) {
        packet_name = "Publication";
        dest_port = "destPortPublisher";
        out_gate = "outLlPublisher";
        chunk_length = transport_packet->getMsg_publisher().getObj_byte_size() * transport_packet->getMsg_publisher().getN_objects();
    }
    else if(type == MessageType::MANEUVER) {
        packet_name = "Maneuver";
        dest_port = "destPortManeuver";
        out_gate = "outLlManeuver";
        chunk_length = par("sizeManeuverMsg").intValue();
    }
    else if(type == MessageType::MEMBER_STATUS) {
        packet_name = "MemberReport";
        dest_port = "destPortMemberReport";
        out_gate = "outLlMemberReport";
        chunk_length = par("sizeMemberReportMsg").intValue();
    }
    else if(type == MessageType::ORCHESTRATION) {
        packet_name = "Orchestration";
        dest_port = "destPortOrchestration";
        out_gate = "outLlMemberControl";
        chunk_length = par("sizeOrchestrationMsg").intValue();
    }

    MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));
    int destination_cluster = (int) binder->getNextHop((MacNodeId) transport_packet->getDst_mac_id());
    transport_packet->setDst_cluster(destination_cluster);
    ClusterDevice target_device = membership_control->clusterMatch(destination_cluster);
    if(target_device == ClusterDevice::MASTER_DEVICE) {
        // the destination node is within the same cluster as this node
        // this cluster is also being managed by this node
        // Use the destination mac id as next hop
        transport_packet->setHop_cluster(destination_cluster);
        transport_packet->setHop_mac_id(transport_packet->getDst_mac_id());
    }
    else if(target_device == ClusterDevice::MEMBER_DEVICE) {
        // the destination node is within the same cluster as this node
        // this node is of type member in this cluster
        // Use the mac id of this member's manager as next hop
        transport_packet->setHop_cluster(destination_cluster);
        transport_packet->setHop_mac_id(membership_control->getManagerID());
    }
    else if(target_device == ClusterDevice::GATEWAY_DEVICE) {
        // the destination node is within the same cluster as this node
        // this node is of type gateway in this cluster
        // Use the mac id of this gateway's manager as next hop
        transport_packet->setHop_cluster(destination_cluster);
        transport_packet->setHop_mac_id(membership_control->getGwManagerID());
    }
    else {
        // There is no direct route to the destination from this node
        // If this node is a master, forward the message to the node having a gateway to the next closest cluster id
        // If not forward the message to either this member's manager or this gateway's manager depending on which cluster id is closest

        // TODO
    }

    // Prepare and send inet Packet
    hop_mac_id = transport_packet->getHop_mac_id();
    inet::Packet *packet = new inet::Packet(packet_name.c_str());
    packet->insertAtBack(inet::makeShared<TransportPacket>(*transport_packet));
    omnetpp::cModule *destination_module = getSimulation()->getModule(binder->getOmnetId((MacNodeId) hop_mac_id));
    inet::L3Address destination_address = inet::L3AddressResolver().resolve(destination_module->getFullName());
    auto addressReq = packet->addTagIfAbsent<inet::L3AddressReq>();
    addressReq->setDestAddress(destination_address);
    auto portReq = packet->addTagIfAbsent<inet::L4PortReq>();
    portReq->setDestPort(par(dest_port.c_str()).intValue());
    send(packet, out_gate.c_str());
}

void RoutingControl::forwardToBackend(TransportPacket *transport_packet, MessageType type) {
    if(type == MessageType::PUBLICATION) {
        ObjectList *msg_publisher = transport_packet->getMsg_publisher().dup();
        send(msg_publisher, "outBkndPublisher");
    }
    else if(type == MessageType::MEMBER_STATUS) {
        MemberStatus *msg_member_status = transport_packet->getMsg_member_status().dup();
        send(msg_member_status, "outBkndMemberReport");
    }

    delete transport_packet;
}

} // namespace convoy_architecture
