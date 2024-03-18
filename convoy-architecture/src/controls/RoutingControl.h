
#ifndef __CONVOY_ARCHITECTURE_ROUTINGCONTROL_H_
#define __CONVOY_ARCHITECTURE_ROUTINGCONTROL_H_

#include <omnetpp.h>

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class RoutingControl : public omnetpp::cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
};

} // namespace convoy_architecture
#endif
