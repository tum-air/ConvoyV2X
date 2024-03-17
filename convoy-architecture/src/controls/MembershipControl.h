
#ifndef __CONVOY_ARCHITECTURE_MEMBERSHIPCONTROL_H_
#define __CONVOY_ARCHITECTURE_MEMBERSHIPCONTROL_H_

#include <omnetpp.h>

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class MembershipControl : public omnetpp::cSimpleModule
{
protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
};

} // namespace convoy_architecture

#endif
