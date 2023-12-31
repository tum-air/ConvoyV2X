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

#ifndef __CONVOY_ARCHITECTURE_WRAPPERNODE_H_
#define __CONVOY_ARCHITECTURE_WRAPPERNODE_H_

#include <omnetpp.h>
#include "common/defs.h"
#include "veins_inet/VeinsInetMobility.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class WrapperNode : public omnetpp::cSimpleModule
{
  private:
    StationType _station_type;
    omnetpp::cMessage* _setup_trigger_ccs_agent;
    omnetpp::cModule* _ccs_agent;
    inet::IMobility* _mobility_module;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    void setupCCSAgent();
    ConvoyDirection getInitialNodeDirection();
  public:
    WrapperNode();
    ~WrapperNode();
    StationType getStationType();
    inet::Coord getCurrentLocation();
};
} // namespace convoy_architecture
#endif
