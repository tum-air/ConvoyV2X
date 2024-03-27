
#include "MembershipControl.h"

namespace convoy_architecture {

Define_Module(MembershipControl);

void MembershipControl::initialize()
{
    // TODO - Generated method body
}

void MembershipControl::handleMessage(omnetpp::cMessage *msg)
{
    // TODO - Generated method body
}

const std::vector<Publication>& MembershipControl::readPublishers() const
{
    return _publishers;
}

ClusterDevice MembershipControl::clusterMatch(int cluster) {
    // TODO
    return(ClusterDevice::UNKNOWN_DEVICE);
}
} // namespace convoy_architecture
