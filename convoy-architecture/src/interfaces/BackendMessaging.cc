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
#include "messages/DtwinSub_m.h"

namespace convoy_architecture {

Define_Module(BackendMessaging);

void BackendMessaging::initialize()
{
    _start_event = new omnetpp::cMessage("startEvent");
    scheduleAt(par("startTime").doubleValue(), _start_event);
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
        if(msg->arrivedOn("inMcsDtwinSub") || msg->arrivedOn("inUlDtwinSub"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received dtwin subscription message from mcs agent / upper layer" << std::endl;
            handleDtwinSubFromMcs(msg);
            delete msg;
        }
        else if(msg->arrivedOn("inLlDtwinSub"))
        {
            EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Received subscriber message from lower layer" << std::endl;
            handleDtwinSubFromLl(msg);
            delete msg;
        }
    }
    else if(msg == _start_event)
    {
        EV_INFO << current_time <<" - BackendMessaging::handleMessage(): " << "Starting BMS, registering hop addresses for sensor stations" << std::endl;

        // append sensor station device information
        int n_stations = par("nStations").intValue();
        std::string direction_str = (par("convoyDirection").intValue() == ConvoyDirection::TOP)? std::string("top") : std::string("bot");
        std::string direction_int = std::to_string(par("convoyDirection").intValue());
        for(int index=0; index<n_stations; index++)
        {
            std::string station_id = std::string("d") + direction_int + std::string("rsu[") + std::to_string(index) + std::string("]");
            std::string hop_module = std::string("bk_if_") + direction_str + std::string("[") + std::to_string(index) + std::string("]");
            inet::L3Address hop_address = inet::L3AddressResolver().resolve(hop_module.c_str());
            _dtwin_station_hop.insert(std::pair<std::string, inet::L3Address>(station_id, hop_address));
            EV_INFO << station_id << ": " << hop_address.str() << std::endl;
        }
    }
}

void BackendMessaging::handleDtwinFromUl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    ObjectList *dtwin_pub_msg = omnetpp::check_and_cast<ObjectList *>(msg);
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

    inet::Packet *packet = omnetpp::check_and_cast<inet::Packet *>(msg);
    auto mcs_packet = packet->popAtFront<MCSPacket>();

    // Update station hop information for each dtwin  and forward dtwin message to application layer
    ObjectList *msg_dtwin_pub = mcs_packet->getMsg_dtwin_pub().dup();

    /* Station hop is set using subscriber application instead of dtwin publish application */
    /*
    int n_objects = msg_dtwin_pub->getN_objects();

    std::string station_id = std::string(msg_dtwin_pub->getStation_id());
    auto addressTag = packet->getTag<inet::L3AddressReq>();
    inet::L3Address hop_address = addressTag->getSrcAddress();
    for (int obj_index=0;obj_index<n_objects;obj_index++)
    {
        std::string dtwin_id = std::string(msg_dtwin_pub->getObject_id(obj_index));
        _dtwin_station_hop.insert(std::pair<std::string, inet::L3Address>(dtwin_id, hop_address));
        EV_INFO << current_time <<" - BackendMessaging::handleDtwinFromLl(): dtwin-station-hop " << dtwin_id << ": " << hop_address.str() << std::endl;
    }
    */

    send(msg_dtwin_pub, "outUlDtwin");
}

void BackendMessaging::handleConvoyOrchFromUl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    ConvoyControlService *convoy_orch_msg = omnetpp::check_and_cast<ConvoyControlService *>(msg);

    auto mcs_packet = inet::makeShared<MCSPacket>();

    mcs_packet->setTimestamp(convoy_orch_msg->getTimestamp());
    mcs_packet->setChunkLength(inet::B(par("sizeConvoyOrchMsg").intValue()));
    mcs_packet->setMsg_ccs(*convoy_orch_msg);

    inet::Packet *packet = new inet::Packet("ConvoyOrchestration");

    // Insert message into transfer packet
    packet->insertAtBack(mcs_packet);

    // Set packet destination for next hop address
    std::string destination_node = std::string(convoy_orch_msg->getNode_id());
    inet::L3Address destination_address = _dtwin_station_hop[destination_node];
    auto addressReq = packet->addTagIfAbsent<inet::L3AddressReq>();
    addressReq->setDestAddress(destination_address);
    auto portReq = packet->addTagIfAbsent<inet::L4PortReq>();
    portReq->setDestPort(par("destinationPortConvoyOrch").intValue());

    EV_INFO << current_time <<" - BackendMessaging::handleConvoyOrchFromUl(): sending mcs packet bound for " << destination_node << " through sensor station " << destination_address.str() << std::endl;
    send(packet, "outLlConvoyOrch");
}

void BackendMessaging::handleConvoyOrchFromLl(omnetpp::cMessage *msg)
{
    // Forward the message to the mcs agent
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - BackendMessaging::handleConvoyOrchFromLl(): forwarding convoy orchestration message to local mcs agent" << std::endl;
    send(msg->dup(), "outMcsAgent");
}

void BackendMessaging::handleDtwinSubFromMcs(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    DtwinSub *dtwin_sub_msg = omnetpp::check_and_cast<DtwinSub *>(msg);
    auto mcs_packet = inet::makeShared<MCSPacket>();
    mcs_packet->setTimestamp(dtwin_sub_msg->getTimestamp());
    mcs_packet->setChunkLength(inet::B(par("sizeDtwinSubMsg").intValue()));
    mcs_packet->setMsg_dtwin_sub(*dtwin_sub_msg);

    inet::Packet *packet = new inet::Packet("DtwinSubscription");

    // Insert message into transfer packet
    packet->insertAtBack(mcs_packet);

    // Set packet destination for next hop address
    inet::L3Address destination_address = inet::L3AddressResolver().resolve(par("destinationModuleDtwin"));
    auto addressReq = packet->addTagIfAbsent<inet::L3AddressReq>();
    addressReq->setDestAddress(destination_address);
    auto portReq = packet->addTagIfAbsent<inet::L4PortReq>();
    portReq->setDestPort(par("destinationPortDtwinSub").intValue());

    EV_INFO << current_time <<" - BackendMessaging::handleDtwinSubFromMcs(): sending mcs packet to backend interface device" << std::endl;
    send(packet, "outLlDtwinSub");
}

void BackendMessaging::handleDtwinSubFromLl(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    inet::Packet *packet = omnetpp::check_and_cast<inet::Packet *>(msg);
    auto mcs_packet = packet->popAtFront<MCSPacket>();

    // Update station hop information for each dtwin  and forward dtwin message to application layer
    DtwinSub *msg_dtwin_sub = mcs_packet->getMsg_dtwin_sub().dup();
    auto addressTag = packet->getTag<inet::L3AddressInd>();
    inet::L3Address hop_address = addressTag->getSrcAddress();
    std::string subscriber_id = std::string(msg_dtwin_sub->getSubscriber_id());
    _dtwin_station_hop.insert(std::pair<std::string, inet::L3Address>(subscriber_id, hop_address));

    EV_INFO << current_time <<" - BackendMessaging::handleDtwinSubFromLl(): subscriber-station-hop " << subscriber_id << ": " << hop_address.str() << std::endl;

    send(msg_dtwin_sub, "outUlDtwinSub");
}
} // namespace convoy_architecture
