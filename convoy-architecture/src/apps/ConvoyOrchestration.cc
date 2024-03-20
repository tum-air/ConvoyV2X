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

#include "ConvoyOrchestration.h"
#include "messages/ConvoyControlService_m.h"

namespace convoy_architecture {

Define_Module(ConvoyOrchestration);

ConvoyOrchestration::ConvoyOrchestration()
{
    _start_event = nullptr;
    _update_event = nullptr;
}

ConvoyOrchestration::~ConvoyOrchestration()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
}

void ConvoyOrchestration::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Initializing convoy orchestration application " << std::endl;

    double start_time = par("startTime").doubleValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");
    _dtwin_store = check_and_cast<DtwinStore*>(this->getParentModule()->getSubmodule("dtwinStore"));
    _member_store = check_and_cast<MemberStore*>(this->getParentModule()->getSubmodule("memberStore"));

    if(par("stopTime").doubleValue() > start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < start_time)? start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Scheduled convoy orchestration start for time " << trigger_time << "s" << std::endl;
    }

        EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Initialized convoy orchestration application" << std::endl;
}

void ConvoyOrchestration::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Starting convoy orchestration application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Executing orchestration step" << std::endl;

            _current_state.clear();
            _desired_state.clear();
            estimate_actual_state();
            compute_desired_state();
            enforce_desired_state();
        }
        scheduleAfter(par("updateRate").doubleValue(), _update_event);
    }

    if ((current_time >= par("stopTime").doubleValue()) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Stopping convoy orchestration application" << std::endl;
        cancelEvent(_update_event);
    }
}

void ConvoyOrchestration::estimate_actual_state() {
    /* TODO
    ObjectList* dtwins = _dtwin_store->readFromStore();
    if(dtwins != nullptr)
        delete dtwins;
     */
    const std::vector<Node>& ccs_reports = _member_store->readCCSReports();
    std::copy(std::begin(ccs_reports), std::end(ccs_reports), std::back_inserter(_current_state));
}

void ConvoyOrchestration::compute_desired_state() {
    /* TODO
     * Convoy orchestration strategy
     */
    std::copy(std::begin(_current_state), std::end(_current_state), std::back_inserter(_desired_state));
}

void ConvoyOrchestration::enforce_desired_state() {
    std::vector<ConvoyControlService*> ccs_msg;
    std::for_each(std::begin(_desired_state), std::end(_desired_state), [this] (const Node& n) {
        ConvoyControlService *ccs_msg = new ConvoyControlService();
        ccs_msg->setNode_id(n.name.c_str());
        ccs_msg->setNode_address(n.address);
        ccs_msg->setTimestamp((uint64_t) omnetpp::simTime().raw());
        send(ccs_msg, "out");
    });
}

} // namespace convoy_architecture
