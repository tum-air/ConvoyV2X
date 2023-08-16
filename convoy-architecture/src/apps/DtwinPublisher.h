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

#ifndef __CONVOY_ARCHITECTURE_DTWINPUBLISHER_H_
#define __CONVOY_ARCHITECTURE_DTWINPUBLISHER_H_

#include <omnetpp.h>
#include "stores/DtwinStore.h"
#include "messages/ObjectList_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class DtwinPublisher : public omnetpp::cSimpleModule
{
  private:
    std::string  _publisher_id;
    omnetpp::simtime_t _start_time;
    omnetpp::simtime_t _stop_time;
    omnetpp::simtime_t _update_rate;
    omnetpp::cMessage* _start_event;
    omnetpp::cMessage* _update_event;
    DtwinStore* _dtwin_store;
    ObjectList* readDtwin();
    void sendDtwinMessage(ObjectList *dtwin_message);
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
  public:
    ~DtwinPublisher();
    DtwinPublisher();
};

} // namespace convoy_architecture

#endif
