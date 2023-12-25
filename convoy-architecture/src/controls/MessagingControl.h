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

#ifndef __CONVOY_ARCHITECTURE_MESSAGINGCONTROL_H_
#define __CONVOY_ARCHITECTURE_MESSAGINGCONTROL_H_

#include <omnetpp.h>
#include "inet/common/packet/Packet.h"
#include "packets/MCSPacket_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class MessagingControl : public omnetpp::cSimpleModule
{
private:
    std::set<std::string> _subscriber_id;
    std::map<std::string, int> _subscriber_address;
    std::map<std::string, uint64_t> _subscriber_timestamp;
    std::map<std::string, std::string> _subscriber_roi;
    std::map<std::string, std::string> _subscriber_qos;
    omnetpp::cMessage* _subscriber_expiry_check_event;
public:
    MessagingControl();
    ~MessagingControl();
    enum ApplicationType {DTWIN_SUB, DTWIN_PUB, COOP_MANEUVER};
protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    void handleDtwinSubscriptionMsgFromUl(omnetpp::cMessage *msg);
    void handleDtwinPublicationMsgFromUl(omnetpp::cMessage *msg);
    void handleCoopManMsgFromUl(omnetpp::cMessage *msg);
    void handleDtwinSubscriptionMsgFromLl(omnetpp::cMessage *msg);
    void handleDtwinPublicationMsgFromLl(omnetpp::cMessage *msg);
    void handleCoopManMsgFromLl(omnetpp::cMessage *msg);
    void handleCCSMsg(omnetpp::cMessage *msg);
    bool fovMatch(double ego_x, std::string roi_tag, double rsu_x, double rsu_fov);
    void routeMessageFromLlToDestination(omnetpp::cMessage *msg, ApplicationType app_type);
    void routeMessageFromUlToDestination(omnetpp::cMessage *msg, int destination_id, ApplicationType app_type);
    std::string setDestinationAddress(inet:: Packet *packet, int hop_mac_id, int application_port);
    void trimSubscriberList();
};

} // namespace convoy_architecture
#endif
