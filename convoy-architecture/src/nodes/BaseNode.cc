
#include "BaseNode.h"
#include "controls/RoutingControl.h"

namespace convoy_architecture {

Define_Module(BaseNode);

BaseNode::~BaseNode()
{
    cancelAndDelete(_event);
}

void BaseNode::initialize()
{
    if((StationType) par("stationType").intValue() == StationType::VEHICLE)
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
    _event = new omnetpp::cMessage("setupMembershipControl");
    scheduleAt(omnetpp::simTime(), _event);
}

void BaseNode::handleMessage(omnetpp::cMessage *msg)
{
    if(msg->isSelfMessage()) {
        if(std::string{msg->getName()} == std::string{"setupMembershipControl"}) {
            par("convoyDirection") = (int) getDirection();
            getSubmodule("mcsAgent")->par("convoyDirection") = par("convoyDirection").intValue();
            setupMembershipControl();
        }
    }
}

ConvoyDirection BaseNode::getDirection()
{
    // Work out center line position
    omnetpp::cModule * scenario_config = getSystemModule()->getSubmodule("scenarioConfig");
    double center_y = (scenario_config->par("road_top_start_y").doubleValue() + scenario_config->par("road_bot_start_y").doubleValue()) / 2;

    return ((getCurrentLocation().y <= center_y)? ConvoyDirection::TOP : ConvoyDirection::DOWN);
}

void BaseNode::setupMembershipControl()
{
    std::string module_name {"membershipControl"};
    omnetpp::cModuleType *module_type = omnetpp::cModuleType::get("convoy_architecture.controls.MembershipControl");
    omnetpp::cModule *membership_control_agent = module_type->create(module_name.c_str(), this);

    // Set position
    omnetpp::cDisplayString display_string = membership_control_agent->getDisplayString();
    display_string.setTagArg("p", 0, 400);
    display_string.setTagArg("p", 1, 250);
    display_string.setTagArg("is", 0, "s");
    membership_control_agent->setDisplayString(display_string.str());
    membership_control_agent->setDisplayName(module_name.c_str());

    // Finalize module
    membership_control_agent->finalizeParameters();
    membership_control_agent->buildInside();
    membership_control_agent->scheduleStart(omnetpp::simTime());
    membership_control_agent->callInitialize();

    // Connect to routing control agent for messaging
    RoutingControl *routing_control_agent = check_and_cast<RoutingControl *>(this->getSubmodule("routingControl"));
    routing_control_agent->gate("outUlMemberControl")->connectTo(membership_control_agent->gate("in"));
    membership_control_agent->gate("out")->connectTo(routing_control_agent->gate("inUlMemberReport"));
}

inet::Coord BaseNode::getCurrentLocation()
{
    inet::IMobility* mobility_module = check_and_cast<inet::IMobility*>(getSubmodule("mobility"));
    return(mobility_module->getCurrentPosition());
}

StationType BaseNode::getStationType()
{
    return((StationType) par("stationType").intValue());
}
} // namespace convoy_architecture
