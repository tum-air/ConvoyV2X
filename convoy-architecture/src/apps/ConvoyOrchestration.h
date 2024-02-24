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
#include "controls/ConvoyControl.h"

namespace convoy_architecture {

struct Node {
    std::string name;
    int id_gnb;
    int id_ue;
    int id_gnb_gw;
    int id_ue_gw;
    StationType type;
    ConvoyControl::Role role;
    inet::Coord position;
    double speed;
    double txp;
    double txp_gw;
};

struct Cluster {
    std::string name;
    int id;
    std::vector<Node> nodes;
    double length;
};

struct Convoy {
    std::string name;
    int id;
    std::vector<Cluster> clusters;
    double length;
};

/**
 * TODO - Generated class
 */
class ConvoyOrchestration : public omnetpp::cSimpleModule
{
  private:
    omnetpp::cMessage* _start_event;
    omnetpp::cMessage* _update_event;
    DtwinStore* _dtwin_store;
    ObjectList* _dtwin;
    std::vector<std::string> _orch_ip_node_id;
    std::vector<inet::Coord> _orch_ip_node_pos;
    std::vector<std::string> _orch_op_node_id;
    std::vector<ConvoyControlService*> _orch_op_node_cc;
    ConvoyDirection _convoy_direction;

    /* ----- */
    std::vector<Convoy> _current_state;
    std::vector<Convoy> _desired_state;
    void estimate_actual_state();
    void compute_desired_state();
    void enforce_desired_state();
    /* ----- */

    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;
    void formatInput();
    void computeOutput();
    void transferOutput();
    std::vector<ConvoyControlService*> executeOrchestrationStep(std::vector<inet::Coord>);
  public:
      ~ConvoyOrchestration();
      ConvoyOrchestration();
};

} // namespace convoy_architecture

#endif
