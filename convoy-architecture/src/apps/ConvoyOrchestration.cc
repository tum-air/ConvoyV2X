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
    _dtwin  = nullptr;
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

    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");
    _dtwin_store = check_and_cast<DtwinStore*>(this->getParentModule()->getSubmodule("dtwinStore"));

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Scheduled convoy orchestration start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
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
            EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Reading from dtwin store, executing orchestration step and publishing ccs messages " << std::endl;

            // 1. Read current digital twin from store
            _dtwin = _dtwin_store->readFromStore();

            // 2. Prepare data for orchestration decision, input: vehicle and sensor station positions
            // 3. Perform orchestration step, output: map of node - ccs message
            // 4. Send out ccs messages
        }
        scheduleAfter(_update_rate, _update_event);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Stopping convoy orchestration application" << std::endl;
        cancelEvent(_update_event);
    }
}

} // namespace convoy_architecture
