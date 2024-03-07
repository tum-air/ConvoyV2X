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

#include "SubscriberStore.h"

namespace convoy_architecture {

Define_Module(SubscriberStore);

void SubscriberStore::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - SubscriberStore::initialize(): " << "Initializing subscriber store " << std::endl;

    double subscriber_max_age = par("subscriberMaxAge").doubleValue();
    _subscriber_expiry_check_event = new omnetpp::cMessage("subscriberExpiryCheck");
    scheduleAfter(par("subscriberCheckInterval").doubleValue(), _subscriber_expiry_check_event);

    EV_INFO << current_time <<" - SubscriberStore::initialize(): " << "Initialized subscriber store for station " << this->getParentModule()->getFullName() << std::endl;
}

void SubscriberStore::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Check if the message is triggered internally
    if (msg->isSelfMessage())
    {
        // Check for object-expiry event
        if (msg == _subscriber_expiry_check_event)
        {
            EV_INFO << current_time <<" - SubscriberStore::handleMessage(): " << "Subscriber-expiry check triggered " << std::endl;
            checkSubscriberExpiry();
            scheduleAfter(par("subscriberCheckInterval").doubleValue(), _subscriber_expiry_check_event);
        }
    }
    else if(msg->arrivedOn("in"))
    {
        EV_INFO << current_time <<" - SubscriberStore::handleMessage(): " << "External message received through input gate " << std::endl;
        DtwinSub *msg_dtwin_sub = check_and_cast<DtwinSub *>(msg);
        appendNodeCCSFeedback(msg_dtwin_sub);
        delete msg_dtwin_sub;
    }
}

void SubscriberStore::appendNodeCCSFeedback(DtwinSub *msg) {
    // Edit existing entry if the node already exists, else create a new entry
    std::string node_name = std::string(msg->getCcs_report_name());
    auto it = std::find_if(std::begin(_subscriber_ccs_record), std::end(_subscriber_ccs_record), [&node_name] (Node const& val) {return val.name == node_name;});

    Node subscriber_info {
        node_name,
        msg->getCcs_report_id_gnb(),
        msg->getCcs_report_id_ue(),
        msg->getCcs_report_id_gnb_gw(),
        msg->getCcs_report_id_ue_gw(),
        (StationType) msg->getCcs_report_type(),
        (Role) msg->getCcs_report_role(),
        inet::Coord{msg->getCcs_report_position_x(), msg->getCcs_report_position_y(), msg->getCcs_report_position_z()},
        msg->getCcs_report_speed(),
        msg->getCcs_report_txp(),
        msg->getCcs_report_txp_gw(),
        msg->getCcs_report_direction(),
        msg->getCcs_report_id_convoy(),
        msg->getCcs_report_id_cluster(),
        msg->getTimestamp()
    };

    if(it != std::end(_subscriber_ccs_record))
    {
        (*it) = subscriber_info;
    }
    else
    {
        _subscriber_ccs_record.push_back(subscriber_info);
    }
}

void SubscriberStore::checkSubscriberExpiry() {
    // TODO
}

} // namespace convoy_architecture
