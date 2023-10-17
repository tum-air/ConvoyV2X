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
    _convoy_direction = static_cast<ConvoyDirection>(par("convoyDirection").intValue());

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

            // 2. Prepare data for orchestration decision
            formatInput();

            // 3. Perform orchestration step
            computeOutput();

            // 4. Send out ccs messages
            transferOutput();

            // 5. Clear dtwin data for next cycle
            if(_dtwin != nullptr)
                delete _dtwin;
        }
        scheduleAfter(_update_rate, _update_event);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Stopping convoy orchestration application" << std::endl;
        cancelEvent(_update_event);
    }
}

void ConvoyOrchestration::formatInput()
{
    _orch_ip_node_id.clear();
    _orch_ip_node_pos.clear();

    // prepare data for vehicular objects
    int n_vehicles = _dtwin->getN_objects();
    for(int index=0; index<n_vehicles; index++)
    {
        _orch_ip_node_id.push_back(_dtwin->getObject_id(index));
        inet::Coord pos;
        pos.x = _dtwin->getObject_position_x(index);
        pos.y = _dtwin->getObject_position_y(index);
        _orch_ip_node_pos.push_back(pos);
    }

    // append sensor station device information
    std::string station_base_id = std::string("d") + std::to_string((int)_convoy_direction) + std::string("rsu");
    omnetpp::cModule* system_module = getSimulation()->getSystemModule();
    if(system_module->hasSubmoduleVector(station_base_id.c_str()))
    {
        int n_stations = system_module->getSubmoduleVectorSize(station_base_id.c_str());
        for(int index=0; index<n_stations; index++)
        {
            omnetpp::cModule* station_module = system_module->getSubmodule(station_base_id.c_str(), index);
            inet::IMobility* station_mobility_module = check_and_cast<inet::IMobility*>(station_module->getSubmodule("mobility"));
            inet::Coord pos = station_mobility_module->getCurrentPosition();
            _orch_ip_node_id.push_back(std::string(station_module->getFullName()));
            _orch_ip_node_pos.push_back(pos);
        }
    }
}

void ConvoyOrchestration::computeOutput()
{
    _orch_op_node_id.clear();
    _orch_op_node_cc.clear();

    _orch_op_node_cc = executeOrchestrationStep(_orch_ip_node_pos);
    _orch_op_node_id = _orch_ip_node_id;
}

std::vector<ConvoyControlService*> ConvoyOrchestration::executeOrchestrationStep(std::vector<inet::Coord> node_pos)
{
    std::vector<ConvoyControlService*> orchestration_op;

    // TODO implement the orchestration strategy
    for(int node_index=0; node_index<_orch_ip_node_id.size(); node_index++)
    {
        ConvoyControlService *cc_op = new ConvoyControlService();
        cc_op->setNode_id(_orch_ip_node_id.at(node_index).c_str());
        orchestration_op.push_back(cc_op);
    }

    return(orchestration_op);
}

void ConvoyOrchestration::transferOutput()
{
    for(int node_index=0; node_index<_orch_op_node_cc.size(); node_index++)
        send(_orch_op_node_cc.at(node_index), "out");
}

} // namespace convoy_architecture
