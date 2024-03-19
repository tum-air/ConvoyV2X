
#include "RoutingControl.h"
#include "messages/DtwinSub_m.h"
#include "messages/ObjectList_m.h"
#include "messages/CoopManeuver_m.h"
#include "messages/MemberStatus_m.h"
#include "messages/Orchestration_m.h"

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
    EV_INFO << current_time <<" - RoutingControl::msgHandlerSubscriber(): " << "Received subscriber message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlSubscriber") {
        // Message received from upper layer
        // check_and_cast<DtwinSub *>(msg);
    }
    else {
        // Message received from lower layer
    }
}
void RoutingControl::msghandlerPublisher(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msghandlerPublisher(): " << "Received publisher message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlPublisher") {
        // Message received from upper layer
        // check_and_cast<ObjectList *>(msg)
    }
    else {
        // Message received from lower layer
    }
}
void RoutingControl::msgHandlerManeuver(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerManeuver(): " << "Received maneuver message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlManeuver") {
        // Message received from upper layer
        // check_and_cast<CoopManeuver *>(msg)
    }
    else {
        // Message received from lower layer
    }
}
void RoutingControl::msgHandlerMemberReport(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerMemberReport(): " << "Received member status message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlMemberReport") {
        // Message received from upper layer
        // check_and_cast<MemberStatus *>(msg)
    }
    else {
        // Message received from lower layer
    }
}
void RoutingControl::msgHandlerOrchestration(omnetpp::cMessage *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    std::string arrival_gate = std::string{msg->getArrivalGate()->getFullName()};
    EV_INFO << current_time <<" - RoutingControl::msgHandlerOrchestration(): " << "Received orchestration message through gate " << arrival_gate << std::endl;

    if(arrival_gate == "inUlMemberControl") {
        // Message received from upper layer
        // check_and_cast<Orchestration *>(msg)
    }
    else if(arrival_gate == "inBkndMemberControl") {
        // Message received from backend
    }
    else {
        // Message received from lower layer
    }
}

void RoutingControl::forwardToNetwork(TransportPacket *msg, MessageType type) {
    //
}
void RoutingControl::forwardToBackend(TransportPacket *msg, MessageType type) {
    //
}
} // namespace convoy_architecture
