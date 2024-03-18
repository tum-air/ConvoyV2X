
#ifndef __CONVOY_ARCHITECTURE_PUBLISHER_H_
#define __CONVOY_ARCHITECTURE_PUBLISHER_H_

#include <omnetpp.h>
#include "stores/DtwinStore.h"
#include "messages/ObjectList_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class Publisher : public omnetpp::cSimpleModule
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
    ~Publisher();
    Publisher();
};

} // namespace convoy_architecture

#endif
