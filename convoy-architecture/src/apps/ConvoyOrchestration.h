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

#ifndef __CONVOY_ARCHITECTURE_CONVOYORCHESTRATION_H_
#define __CONVOY_ARCHITECTURE_CONVOYORCHESTRATION_H_

#include <omnetpp.h>
#include "stores/DtwinStore.h"
#include "messages/ObjectList_m.h"
#include "veins_inet/VeinsInetMobility.h"
#include "messages/ConvoyControlService_m.h"
#include "common/defs.h"
#include "stores/SubscriberStore.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class ConvoyOrchestration : public omnetpp::cSimpleModule
{
  private:
    omnetpp::cMessage* _start_event;
    omnetpp::cMessage* _update_event;
    DtwinStore* _dtwin_store;
    SubscriberStore* _subscriber_store;
    std::vector<Node> _current_state;
    std::vector<Node> _desired_state;
    void estimate_actual_state();
    void compute_desired_state();
    void enforce_desired_state();

    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;

  public:
      ~ConvoyOrchestration();
      ConvoyOrchestration();
};

} // namespace convoy_architecture

#endif
