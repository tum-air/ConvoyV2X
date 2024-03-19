
#include "SubscriberStore.h"

namespace convoy_architecture {

Define_Module(SubscriberStore);

void SubscriberStore::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - SubscriberStore::initialize(): " << "Initializing subscriber store " << std::endl;

    _subscriber_expiry_check_event = new omnetpp::cMessage("subscriberExpiryCheck");
    scheduleAfter(par("subscriberCheckInterval").doubleValue(), _subscriber_expiry_check_event);

    EV_INFO << current_time <<" - SubscriberStore::initialize(): " << "Initialized subscriber store for station " << this->getParentModule()->getFullName() << std::endl;
}

void SubscriberStore::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Check if the message is triggered internally
    if (msg->isSelfMessage())
    {
        // Check for object-expiry event
        if (msg == _subscriber_expiry_check_event)
        {
            EV_INFO << current_time <<" - SubscriberStore::handleMessage(): " << "Subscriber-expiry check triggered " << std::endl;
            checkSubscriberExpiry();
            scheduleAfter(par("subscriberCheckInterval").doubleValue(), _subscriber_expiry_check_event);
        }
    }
    else if(msg->arrivedOn("in"))
    {
        EV_INFO << current_time <<" - SubscriberStore::handleMessage(): " << "External message received through input gate " << std::endl;
        DtwinSub *msg_dtwin_sub = check_and_cast<DtwinSub *>(msg);
        delete msg_dtwin_sub;
    }
}

void SubscriberStore::updateSubscriberList(DtwinSub *msg) {
    // Edit existing entry if the node already exists and if the time stamp is more recent, else create a new entry
    Subscription subscriber {
        std::string{msg->getSubscriber_id()},
        msg->getSubscriber_address(),
        std::string{msg->getRoi_tag()},
        std::string{msg->getQos_tag()},
        msg->getTimestamp()
    };

    std::string node_name = subscriber.name;
    auto it = std::find_if(std::begin(_subscriber_record), std::end(_subscriber_record), [&node_name] (Subscription const& val) {return val.name == node_name;});
    if(it != std::end(_subscriber_record)) {
        if((*it).timestamp < subscriber.timestamp)
            (*it) = subscriber;
    }
    else
        _subscriber_record.push_back(subscriber);
}

void SubscriberStore::checkSubscriberExpiry() {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    uint64_t current_nsec = (uint64_t) current_time.raw();
    omnetpp::simtime_t max_age_sec = par("subscriberMaxAge").doubleValue();
    uint64_t max_age_nsec = (uint64_t) max_age_sec.raw();

    _subscriber_record.erase(std::remove_if(std::begin(_subscriber_record), std::end(_subscriber_record),
            [&current_nsec, &max_age_nsec] (Subscription const& val) {return (current_nsec - val.timestamp) >= max_age_nsec;}),
            std::end(_subscriber_record));
}

const std::vector<Subscription>& SubscriberStore::readSubscriptions() const
{
    return _subscriber_record;
}

} // namespace convoy_architecture
