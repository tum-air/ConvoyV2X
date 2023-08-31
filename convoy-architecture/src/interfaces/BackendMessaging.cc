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

#include "BackendMessaging.h"
#include "messages/ObjectList_m.h"
#include "packets/MCSPacket_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

namespace convoy_architecture {

Define_Module(BackendMessaging);

void BackendMessaging::initialize()
{
    // TODO - Generated method body
}

void BackendMessaging::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    if(!msg->isSelfMessage())
    {
        if(msg->arrivedOn("inUl"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received dtwin message from upper layer" << std::endl;
            handleDtwinFromUl(msg);
            delete msg;
        }
        else if(msg->arrivedOn("inLl"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received dtwin message from lower layer" << std::endl;
            handleDtwinFromLl(msg);
            delete msg;
        }
    }
}

void BackendMessaging::handleDtwinFromUl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    ObjectList *dtwin_pub_msg = check_and_cast<ObjectList *>(msg);
    auto mcs_packet = inet::makeShared<MCSPacket>();
    mcs_packet->setTimestamp(dtwin_pub_msg->getTimestamp());
    mcs_packet->setChunkLength(inet::B(dtwin_pub_msg->getObj_byte_size() * dtwin_pub_msg->getN_objects()));
    mcs_packet->setMsg_dtwin_pub(*dtwin_pub_msg);

    inet::Packet *packet = new inet::Packet("DtwinPublication");

    // Insert message into transfer packet
    packet->insertAtBack(mcs_packet);

    // Set packet destination for next hop address
    inet::L3Address destination_address = inet::L3AddressResolver().resolve(par("destinationModule"));
    auto addressReq = packet->addTagIfAbsent<inet::L3AddressReq>();
    addressReq->setDestAddress(destination_address);
    auto portReq = packet->addTagIfAbsent<inet::L4PortReq>();
    portReq->setDestPort(par("destinationPort").intValue());

    EV_INFO << current_time <<" - BackendMessaging::handleDtwinFromUl(): sending mcs packet to backend interface device" << std::endl;
    send(packet, "outLl");
}

void BackendMessaging::handleDtwinFromLl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    inet::Packet *packet = check_and_cast<inet::Packet *>(msg);
    auto mcs_packet = packet->popAtFront<MCSPacket>();

    // Forward dtwin message to application layer
    ObjectList *msg_dtwin_pub = mcs_packet->getMsg_dtwin_pub().dup();
    send(msg_dtwin_pub, "outUl");
}
} // namespace convoy_architecture
