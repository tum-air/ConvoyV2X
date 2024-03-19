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

#ifndef __CONVOY_ARCHITECTURE_MEMBERSTORE_H_
#define __CONVOY_ARCHITECTURE_MEMBERSTORE_H_

#include <omnetpp.h>
#include "common/defs.h"
#include "messages/MemberStatus_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
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
    }
    const std::vector<Node>& readCCSReports() const;
};

} // namespace convoy_architecture


#endif
