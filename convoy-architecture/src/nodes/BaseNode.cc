
#include "BaseNode.h"

namespace convoy_architecture {

Define_Module(BaseNode);

void BaseNode::initialize()
{
    // TODO - Generated method body
}

void BaseNode::handleMessage(omnetpp::cMessage *msg)
{
    // TODO - Generated method body
}

inet::Coord BaseNode::getCurrentLocation()
{
    inet::IMobility* mobility_module = check_and_cast<inet::IMobility*>(getSubmodule("mobility"));
    return(mobility_module->getCurrentPosition());
}

} // namespace convoy_architecture
