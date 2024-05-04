
#ifndef __CONVOY_ARCHITECTURE_PUBLISHER_H_
#define __CONVOY_ARCHITECTURE_PUBLISHER_H_

#include <omnetpp.h>
#include "messages/ObjectList_m.h"

namespace convoy_architecture {

class Publisher : public omnetpp::cSimpleModule
{
  private:
    std::string  _publisher_id;
    omnetpp::simtime_t _start_time;
    omnetpp::simtime_t _stop_time;
    omnetpp::simtime_t _update_rate;
    omnetpp::cMessage* _start_event {nullptr};
    omnetpp::cMessage* _update_event {nullptr};
    ObjectList* readDtwin();
    void sendDtwinMessage(ObjectList *dtwin_message);
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
  public:
    ~Publisher();
};

} // namespace convoy_architecture

#endif
