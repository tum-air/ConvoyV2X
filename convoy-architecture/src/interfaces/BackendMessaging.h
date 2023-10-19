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

#ifndef __CONVOY_ARCHITECTURE_BACKENDMESSAGING_H_
#define __CONVOY_ARCHITECTURE_BACKENDMESSAGING_H_

#include <omnetpp.h>

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class BackendMessaging : public omnetpp::cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    void handleDtwinFromUl(omnetpp::cMessage *msg);
    void handleDtwinFromLl(omnetpp::cMessage *msg);
    void handleConvoyOrchFromUl(omnetpp::cMessage *msg);
    void handleConvoyOrchFromLl(omnetpp::cMessage *msg);

  private:
    std::map<std::string, std::string> _dtwin_station_hop;
};

} // namespace convoy_architecture
#endif
