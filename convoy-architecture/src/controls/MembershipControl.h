
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
    int _id_gnb {0};
    int _id_gnb_gw {0};
    int _address_pr {0};
    int _address_gw {0};

public:
    bool isInitialized() {return (_control_state == MemberControlState::INITIALIZED);}
    const std::vector<Publication>& readPublishers() const;
    int getManagerID() {return _id_gnb;}
    int getGwManagerID() {return _id_gnb_gw;}
    bool addressMatch(int address) {return (address==_address_pr || address==_address_gw);}
    ClusterDevice clusterMatch(int cluster);
};

} // namespace convoy_architecture

#endif
