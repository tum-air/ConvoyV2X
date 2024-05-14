
#ifndef __CONVOY_ARCHITECTURE_MEMBERSTORE_H_
#define __CONVOY_ARCHITECTURE_MEMBERSTORE_H_

#include <omnetpp.h>
#include "common/defs.h"
#include "messages/MemberStatus_m.h"

namespace convoy_architecture {

class MemberStore : public omnetpp::cSimpleModule
{
  private:
    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;
    void checkMemberExpiry();
    void updateNodeCCSReports(MemberStatus *msg);

    omnetpp::cMessage *_member_expiry_check_event{nullptr};
    std::vector<Node> _member_ccs_record;

  public:
    ~MemberStore() {
        if (_member_expiry_check_event != nullptr)
            cancelAndDelete(_member_expiry_check_event);
        _member_expiry_check_event = nullptr;
    }
    const std::vector<Node>& readCCSReports() const;
};

} // namespace convoy_architecture


#endif
