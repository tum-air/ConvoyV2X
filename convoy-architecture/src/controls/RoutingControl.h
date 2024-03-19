
#ifndef __CONVOY_ARCHITECTURE_ROUTINGCONTROL_H_
#define __CONVOY_ARCHITECTURE_ROUTINGCONTROL_H_

#include <omnetpp.h>
#include "common/defs.h"
#include "inet/common/packet/Packet.h"
#include "packets/TransportPacket_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class RoutingControl : public omnetpp::cSimpleModule
{
  private:
    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;

    void msgHandlerSubscriber(omnetpp::cMessage *msg);
    void msghandlerPublisher(omnetpp::cMessage *msg);
    void msgHandlerManeuver(omnetpp::cMessage *msg);
    void msgHandlerMemberReport(omnetpp::cMessage *msg);
    void msgHandlerOrchestration(omnetpp::cMessage *msg);

    void forwardToNextHop(omnetpp::cMessage *application_message, int destination, MessageType type);
    void forwardToNextHop(inet::Packet *network_packet, MessageType type);
    void forwardToNetwork(TransportPacket *transport_packet, MessageType type);
    void forwardToBackend(TransportPacket *transport_packet, MessageType type);
};

} // namespace convoy_architecture
#endif
