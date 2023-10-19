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
#include "messages/ConvoyControlService_m.h"
#include "common/defs.h"

namespace convoy_architecture {

Define_Module(BackendMessaging);

void BackendMessaging::initialize()
{
    // append sensor station device information
    int n_stations = par("nStations").intValue();
    for(int index=0; index<n_stations; index++)
    {
        std::string station_id = std::string("d") + std::to_string(par("convoyDirection").intValue()) + std::string("rsu[") + std::to_string(index) + std::string("]");
        _dtwin_station_hop.insert(std::pair<std::string, std::string>(station_id, station_id));
    }
}

void BackendMessaging::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    if(!msg->isSelfMessage())
    {
        if(msg->arrivedOn("inUlDtwin"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received dtwin message from upper layer" << std::endl;
            handleDtwinFromUl(msg);
            delete msg;
        }
        else if(msg->arrivedOn("inLlDtwin"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received dtwin message from lower layer" << std::endl;
            handleDtwinFromLl(msg);
            delete msg;
        }
        else if(msg->arrivedOn("inUlConvoyOrch"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received convoy orchestration message from upper layer" << std::endl;
            handleConvoyOrchFromUl(msg);
            delete msg;
        }
        else if(msg->arrivedOn("inLlConvoyOrch"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received convoy orchestration message from lower layer" << std::endl;
            handleConvoyOrchFromLl(msg);
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
    inet::L3Address destination_address = inet::L3AddressResolver().resolve(par("destinationModuleDtwin"));
    auto addressReq = packet->addTagIfAbsent<inet::L3AddressReq>();
    addressReq->setDestAddress(destination_address);
    auto portReq = packet->addTagIfAbsent<inet::L4PortReq>();
    portReq->setDestPort(par("destinationPortDtwin").intValue());

    EV_INFO << current_time <<" - BackendMessaging::handleDtwinFromUl(): sending mcs packet to backend interface device" << std::endl;
    send(packet, "outLlDtwin");
}

void BackendMessaging::handleDtwinFromLl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    inet::Packet *packet = check_and_cast<inet::Packet *>(msg);
    auto mcs_packet = packet->popAtFront<MCSPacket>();

    // Update station hop information for each dtwin  and forward dtwin message to application layer
    ObjectList *msg_dtwin_pub = mcs_packet->getMsg_dtwin_pub().dup();
    int n_objects = msg_dtwin_pub->getN_objects();
    std::string station_id = std::string(msg_dtwin_pub->getStation_id());
    for (int obj_index=0;obj_index<n_objects;obj_index++)
    {
        std::string dtwin_id = std::string(msg_dtwin_pub->getObject_id(obj_index));
        _dtwin_station_hop.insert(std::pair<std::string, std::string>(dtwin_id, station_id));
        EV_INFO << current_time <<" - BackendMessaging::handleDtwinFromLl(): dtwin-station-hop " << dtwin_id << "-" << station_id << std::endl;
    }

    send(msg_dtwin_pub, "outUlDtwin");
}

void BackendMessaging::handleConvoyOrchFromUl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    ConvoyControlService *convoy_orch_msg = check_and_cast<ConvoyControlService *>(msg);
    auto mcs_packet = inet::makeShared<MCSPacket>();

    mcs_packet->setTimestamp(convoy_orch_msg->getTimestamp());
    mcs_packet->setChunkLength(inet::B(par("sizeConvoyOrchMsg").intValue()));
    mcs_packet->setMsg_ccs(*convoy_orch_msg);

    inet::Packet *packet = new inet::Packet("ConvoyOrchestration");

    // Insert message into transfer packet
    packet->insertAtBack(mcs_packet);

    // Set packet destination for next hop address
    std::string destination_node = std::string(convoy_orch_msg->getNode_id());
    std::string next_hop = _dtwin_station_hop[destination_node];
    inet::L3Address destination_address = inet::L3AddressResolver().resolve(next_hop.c_str());
    auto addressReq = packet->addTagIfAbsent<inet::L3AddressReq>();
    addressReq->setDestAddress(destination_address);
    auto portReq = packet->addTagIfAbsent<inet::L4PortReq>();
    portReq->setDestPort(par("destinationPortConvoyOrch").intValue());

    EV_INFO << current_time <<" - BackendMessaging::handleConvoyOrchFromUl(): sending mcs packet bound for " << destination_node << " through sensor station " << next_hop << std::endl;
    send(packet, "outLlConvoyOrch");
}

void BackendMessaging::handleConvoyOrchFromLl(omnetpp::cMessage *msg)
{
    // Forward the message to the mcs agent
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - BackendMessaging::handleConvoyOrchFromLl(): forwarding convoy orchestration message to local mcs agent" << std::endl;
    send(msg->dup(), "outMcsAgent");
}
} // namespace convoy_architecture
