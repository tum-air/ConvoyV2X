
#ifndef __CONVOY_ARCHITECTURE_CONVOYORCHESTRATION_H_
#define __CONVOY_ARCHITECTURE_CONVOYORCHESTRATION_H_

#include <omnetpp.h>
#include "common/defs.h"

namespace convoy_architecture {

class ConvoyOrchestration : public omnetpp::cSimpleModule
{
  private:
    omnetpp::cMessage* _start_event{nullptr};
    omnetpp::cMessage* _update_event{nullptr};
    std::vector<Node> _current_state;
    std::vector<Node> _desired_state;
    void estimate_actual_state();
    void compute_desired_state();
    void enforce_desired_state();

    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;

  public:
      ~ConvoyOrchestration();
};

} // namespace convoy_architecture

#endif
