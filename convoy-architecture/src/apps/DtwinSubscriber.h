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

#ifndef __CONVOY_ARCHITECTURE_DTWINSUBSCRIBER_H_
#define __CONVOY_ARCHITECTURE_DTWINSUBSCRIBER_H_

#include <omnetpp.h>
#include "messages/ObjectList_m.h"
#include "messages/DtwinSub_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class DtwinSubscriber : public omnetpp::cSimpleModule
{
  private:
    std::string  _subscriber_id;
    std::string  _roi_tag;
    std::string  _qos_tag;
    std::string  _subscriber_type;
    omnetpp::simtime_t _start_time;
    omnetpp::simtime_t _stop_time;
    omnetpp::simtime_t _update_rate;
    omnetpp::cMessage* _start_event;
    omnetpp::cMessage* _update_event;
    void sendDtwinSubscriptionMessage();
    void receiveDtwinMessage(ObjectList *dtwin_message);
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
  public:
    ~DtwinSubscriber();
    DtwinSubscriber();
    void updateROI(std::string roi_tag);
    void updateQOS(std::string qos_tag);
    void updateSubscriberID(std::string subscriber_id);
    void appendCCSReport(DtwinSub *msg);
};

} // namespace convoy_architecture

#endif
