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

#include "WrapperNode.h"

namespace convoy_architecture {

Define_Module(WrapperNode);

WrapperNode::WrapperNode()
{
    _setup_trigger_ccs_agent = nullptr;
}
WrapperNode::~WrapperNode()
{
    cancelAndDelete(_setup_trigger_ccs_agent);
}

void WrapperNode::initialize()
{
    _station_type = (StationType) par("stationType").intValue();
    _mobility_module = check_and_cast<inet::IMobility*>(getSubmodule("mobility"));

    if(_station_type == StationType::VEHICLE)
    {
        omnetpp::cDisplayString display_string = getDisplayString();
        display_string.removeTag("i");
        display_string.setTagArg("b", 0, 4);
        display_string.setTagArg("b", 1, 2);
        display_string.setTagArg("b", 2, "rect");
        display_string.setTagArg("b", 3, "white");
        display_string.setTagArg("b", 4, "white");
        setDisplayString(display_string.str());
    }

    par("stationID") = getFullName();

    // Setup trigger to initialize associated modules
    _setup_trigger_ccs_agent = new omnetpp::cMessage("setup_ccs_agent");
    scheduleAt(omnetpp::simTime(), _setup_trigger_ccs_agent);
}

void WrapperNode::handleMessage(omnetpp::cMessage *msg)
{
    if(msg == _setup_trigger_ccs_agent)
    {
        // Initial setup
        par("convoyDirection") = (int) getInitialNodeDirection();
        getSubmodule("mcsAgent")->par("convoyDirection") = par("convoyDirection").intValue();
        setupCCSAgent();
    }
}

void WrapperNode::setupCCSAgent()
{
    // Setup and initialize CCS agent
    std::string module_type_name = std::string("convoy_architecture.controls.ConvoyControl");
    std::string module_name = std::string("ccsAgent");
    omnetpp::cModuleType *module_type = omnetpp::cModuleType::get(module_type_name.c_str());

    // Create the sensor station module
    _ccs_agent = module_type->create(module_name.c_str(), this);

    // Set position
    omnetpp::cDisplayString display_string = _ccs_agent->getDisplayString();
    display_string.setTagArg("p", 0, 325);
    display_string.setTagArg("p", 1, 250);
    display_string.setTagArg("is", 0, "s");
    _ccs_agent->setDisplayString(display_string.str());
    _ccs_agent->setDisplayName(module_name.c_str());

    // Finalize module
    _ccs_agent->par("convoyDirection") = (int) getInitialNodeDirection();
    _ccs_agent->par("stationType") = (int) _station_type;
    _ccs_agent->finalizeParameters();
    _ccs_agent->buildInside();
    _ccs_agent->scheduleStart(omnetpp::simTime());
    _ccs_agent->callInitialize();
}

ConvoyDirection WrapperNode::getInitialNodeDirection()
{
    // work out center line position
    omnetpp::cModule * world_scenario_module = getSystemModule()->getSubmodule("scenarioConfig");
    double center_y = (world_scenario_module->par("road_top_start_y").doubleValue() + world_scenario_module->par("road_bot_start_y").doubleValue()) / 2;

    ConvoyDirection node_direction = (getCurrentLocation().y <= center_y)? ConvoyDirection::TOP : ConvoyDirection::DOWN;
    return (node_direction);
}

StationType WrapperNode::getStationType()
{
    return(_station_type);
}

inet::Coord WrapperNode::getCurrentLocation()
{
    return(_mobility_module->getCurrentPosition());
}

} // namespace convoy_architecture
