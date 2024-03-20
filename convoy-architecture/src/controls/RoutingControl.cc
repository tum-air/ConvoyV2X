
#include "RoutingControl.h"
#include "messages/DtwinSub_m.h"
#include "messages/ObjectList_m.h"
#include "messages/CoopManeuver_m.h"
#include "messages/MemberStatus_m.h"
#include "messages/Orchestration_m.h"
#include "controls/MembershipControl.h"
#include "apps/Localizer.h"
#include "stores/SubscriberStore.h"

namespace convoy_architecture {

Define_Module(RoutingControl);

void RoutingControl::initialize() {
    // TODO - Generated method body
}

void RoutingControl::handleMessage(omnetpp::cMessage *msg) {
    if(msg->arrivedOn("inUlSubscriber") || msg->arrivedOn("inLlSubscriber"))
        msgHandlerSubscriber(msg);

    else if(msg->arrivedOn("inUlPublisher") || msg->arrivedOn("inLlPublisher"))
        msghandlerPublisher(msg);

    else if(msg->arrivedOn("inUlManeuver") || msg->arrivedOn("inLlManeuver"))
        msgHandlerManeuver(msg);

    else if(msg->arrivedOn("inUlMemberReport") || msg->arrivedOn("inLlMemberReport"))
        msgHandlerMemberReport(msg);

    else if(msg->arrivedOn("inUlMemberControl") || msg->arrivedOn("inLlMemberControl") || msg->arrivedOn("inBkndMemberControl"))
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
                    forwardToNextHop(msg->dup(), p.id, MessageType::SUBSCRIPTION);
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
        forwardToNextHop(check_and_cast<inet::Packet *>(msg), MessageType::SUBSCRIPTION);
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
                    // Send out the dtwin publications
                    std::for_each(std::begin(subscribers), std::end(subscribers), [this, msg] (Subscription const& s) {
                        forwardToNextHop(msg->dup(), s.id, MessageType::PUBLICATION);
                    });
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
        forwardToNextHop(check_and_cast<inet::Packet *>(msg), MessageType::PUBLICATION);
}

void RoutingControl::msgHandlerManeuver(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerManeuver(): " << "Received message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlManeuver") {
        if(getParentModule()->hasSubmoduleVector("membershipControl")) {
            MembershipControl* membership_control = check_and_cast<MembershipControl *>(getParentModule()->getSubmodule("membershipControl"));
            if(membership_control->isInitialized()) {
                CoopManeuver *msg_maneuver = check_and_cast<CoopManeuver *>(msg);
                forwardToNextHop(msg, msg_maneuver->getPartner_address(), MessageType::MANEUVER);
            }
            else
                delete msg;
        }
        else
            delete msg;
    }
    else
        forwardToNextHop(check_and_cast<inet::Packet *>(msg), MessageType::MANEUVER);
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
                forwardToNextHop(msg->dup(), p.id, MessageType::MEMBER_STATUS);
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
            forwardToNextHop(check_and_cast<inet::Packet *>(msg), MessageType::MEMBER_STATUS);
    }
}

void RoutingControl::msgHandlerOrchestration(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerOrchestration(): " << "Received orchestration message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlMemberControl") {
        // Message received from upper layer
        Orchestration * msg_orchestration = check_and_cast<Orchestration *>(msg);
        forwardToNextHop(msg, msg_orchestration->getNode_address(), MessageType::ORCHESTRATION);
    }
    else if(arrival_gate == "inBkndMemberControl")
        forwardToNetwork(check_and_cast<TransportPacket *>(msg), MessageType::ORCHESTRATION);
    else {
        // Message received from lower layer
        forwardToNextHop(check_and_cast<inet::Packet *>(msg), MessageType::ORCHESTRATION);
    }
}

void RoutingControl::forwardToNextHop(omnetpp::cMessage *application_message, int destination, MessageType type) {
    //
}

void RoutingControl::forwardToNextHop(inet::Packet *network_packet, MessageType type) {
    //
}

void RoutingControl::forwardToNetwork(TransportPacket *msg, MessageType type) {
    //
}

void RoutingControl::forwardToBackend(TransportPacket *msg, MessageType type) {
    //
}

} // namespace convoy_architecture
