
#ifndef __CONVOY_ARCHITECTURE_MEMBERSHIPCONTROL_H_
#define __CONVOY_ARCHITECTURE_MEMBERSHIPCONTROL_H_

#include <omnetpp.h>
#include "common/defs.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class MembershipControl : public omnetpp::cSimpleModule
{
private:
    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;

    MemberControlState _control_state {MemberControlState::INITIALIZING};

    std::vector<Publication> _publishers;
    int _id_gnb;

public:
    bool isInitialized() {return (_control_state == MemberControlState::INITIALIZED);}
    const std::vector<Publication>& readPublishers() const;
    int getManagerID() {return _id_gnb;}
};

} // namespace convoy_architecture

#endif
