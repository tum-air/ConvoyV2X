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

#include "DtwinSubscriber.h"
#include "controls/ConvoyControl.h"
#include "common/binder/Binder.h"
#include "veins_inet/VeinsInetMobility.h"
#include "nodes/WrapperNode.h"

namespace convoy_architecture {

Define_Module(DtwinSubscriber);

DtwinSubscriber::DtwinSubscriber()
{
    _start_event = nullptr;
    _update_event = nullptr;
}

DtwinSubscriber::~DtwinSubscriber()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
}

void DtwinSubscriber::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - DtwinSubscriber::initialize(): " << "Initializing dtwin subscriber application " << std::endl;

    _subscriber_id = par("subscriberID").stdstringValue();
    _roi_tag = par("roiTag").stdstringValue();
    _qos_tag = par("qosTag").stdstringValue();
    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();
    _subscriber_type = par("subscriberType").stdstringValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - DtwinSubscriber::initialize(): " << "Scheduled dtwin subscriber application start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - DtwinSubscriber::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
    }

    EV_INFO << current_time <<" - DtwinSubscriber::initialize(): " << "Initialized dtwin subscriber application for station " << this->getParentModule()->getFullName() << std::endl;
}

void DtwinSubscriber::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    updateSubscriberID(getParentModule()->getFullName());

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - DtwinSubscriber::handleMessage(): " << "Starting dtwin subscriber application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - DtwinSubscriber::handleMessage(): " << "Publishing dtwin subscription message" << std::endl;
            this->sendDtwinSubscriptionMessage();
        }
        scheduleAfter(_update_rate, _update_event);
    }
    else
    {
        EV_INFO << current_time <<" - DtwinSubscriber::handleMessage(): " << "Receiving dtwin message" << std::endl;
        ObjectList *dtwin_message = omnetpp::check_and_cast<ObjectList *>(msg);
        this->receiveDtwinMessage(dtwin_message);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - DtwinSubscriber::handleMessage(): " << "Stopping dtwin subscriber application" << std::endl;
        cancelEvent(_update_event);
    }
}

void DtwinSubscriber::sendDtwinSubscriptionMessage()
{
    DtwinSub *sub_message = new DtwinSub();
    sub_message->setTimestamp((uint64_t) omnetpp::simTime().raw());
    sub_message->setSubscriber_id(_subscriber_id.c_str());
    sub_message->setRoi_tag(_roi_tag.c_str());
    sub_message->setQos_tag(_qos_tag.c_str());
    sub_message->setSubscriber_type(_subscriber_type.c_str());

    appendCCSReport(sub_message);
    this->send(sub_message, "out_sub");
}

void DtwinSubscriber::receiveDtwinMessage(ObjectList *dtwin_message)
{
    this->send(dtwin_message, "out_dtwin");
}

void DtwinSubscriber::updateSubscriberID(std::string subscriber_id)
{
    _subscriber_id = subscriber_id;
}

void DtwinSubscriber::updateROI(std::string roi_tag)
{
    _roi_tag = roi_tag;
}

void DtwinSubscriber::updateQOS(std::string qos_tag)
{
    _qos_tag = qos_tag;
}

void DtwinSubscriber::appendCCSReport(DtwinSub *msg)
{
    ConvoyControl *ccs_agent_module = check_and_cast<ConvoyControl *>(getParentModule()->getSubmodule("ccsAgent"));
    Binder *binder = check_and_cast<Binder *>(getSystemModule()->getSubmodule("binder"));

    if(ccs_agent_module != nullptr)
    {
        if(ccs_agent_module->getAgentStatus() != ConvoyControl::AgentStatus::STATUS_INIT_SCANNING)
        {
            msg->setCcs_report_name(_subscriber_id.c_str());
            msg->setCcs_report_direction(ccs_agent_module->getConvoyDirection());
            msg->setCcs_report_convoy_id(ccs_agent_module->getConvoyIdCCS());
            msg->setCcs_report_cluster_id(ccs_agent_module->getClusterIdCCS());
            switch(ccs_agent_module->getClusterRole())
            {
                case Role::MANAGER:
                    msg->setCcs_report_id_ue(ccs_agent_module->getManagerId());
                    msg->setCcs_report_id_gnb(ccs_agent_module->getManagerId());
                    break;
                case Role::MEMBER:
                    msg->setCcs_report_id_ue(ccs_agent_module->getMemberId());
                    msg->setCcs_report_id_gnb(binder->getNextHop(ccs_agent_module->getMemberId()));
                    break;
                case Role::GATEWAY:
                    msg->setCcs_report_id_ue(ccs_agent_module->getMemberId());
                    msg->setCcs_report_id_gnb(binder->getNextHop(ccs_agent_module->getMemberId()));
                    msg->setCcs_report_id_ue_gw(ccs_agent_module->getGatewayId());
                    msg->setCcs_report_id_gnb_gw(binder->getNextHop(ccs_agent_module->getGatewayId()));
                default:
                    break;

            }

            WrapperNode* wrapper_node = check_and_cast<WrapperNode*>(getParentModule());
            msg->setCcs_report_type(wrapper_node->getStationType());
            msg->setCcs_report_role(ccs_agent_module->getClusterRole());

            inet::Coord object_velocity;
            inet::Coord object_position;
            if(wrapper_node->getStationType() == StationType::RSU) {
                inet::IMobility *object_mobility_module = check_and_cast<inet::IMobility*>(getParentModule()->getSubmodule("mobility"));
                object_position = object_mobility_module->getCurrentPosition();
                object_velocity = object_mobility_module->getCurrentVelocity();
            }
            else {
                veins::VeinsInetMobility* object_mobility_module = check_and_cast<veins::VeinsInetMobility*>(getParentModule()->getSubmodule("mobility"));
                object_position = object_mobility_module->getCurrentPosition();
                object_velocity = object_mobility_module->getCurrentVelocity();
            }
            msg->setCcs_report_position_x(object_position.x);
            msg->setCcs_report_position_y(object_position.y);
            msg->setCcs_report_position_z(object_position.z);
            msg->setCcs_report_speed(object_velocity.length());

            msg->setCcs_report_txp(ccs_agent_module->getTxp());
            msg->setCcs_report_txp_gw(ccs_agent_module->getTxpGw());

            msg->setCcs_report_id_convoy(ccs_agent_module->getConvoyIdCCS());
            msg->setCcs_report_id_cluster(ccs_agent_module->getClusterIdCCS());
        }
    }
}

} // namespace convoy_architecture
