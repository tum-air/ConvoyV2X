
#ifndef __CONVOY_ARCHITECTURE_SUBSCRIBERSTORE_H_
#define __CONVOY_ARCHITECTURE_SUBSCRIBERSTORE_H_

#include <omnetpp.h>
#include "common/defs.h"
#include "messages/DtwinSub_m.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class SubscriberStore : public omnetpp::cSimpleModule
{
  private:
    void initialize() override;
    void handleMessage(omnetpp::cMessage *msg) override;
    void checkSubscriberExpiry();
    void updateSubscriberList(DtwinSub *msg);

    omnetpp::cMessage *_subscriber_expiry_check_event{nullptr};
    std::vector<Subscription> _subscriber_record;

  public:
    ~SubscriberStore() {
        if (_subscriber_expiry_check_event != nullptr)
            cancelAndDelete(_subscriber_expiry_check_event);
    }
    const std::vector<Subscription>& readSubscriptions() const;
};

} // namespace convoy_architecture

#endif
