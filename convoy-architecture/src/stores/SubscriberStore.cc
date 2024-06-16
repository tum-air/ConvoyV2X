
#include "SubscriberStore.h"
#include "DtwinStore.h"

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
        updateSubscriberList(msg_dtwin_sub);
        delete msg;
    }
}

void SubscriberStore::updateSubscriberList(DtwinSub *msg) {
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Update address of subscriber in dtwin store
    std::string subscriber_name{msg->getSubscriber_id()};
    int subscriber_address{msg->getSubscriber_address()};
    std::string subscriber_type{msg->getSubscriber_type()};
    DtwinStore *dtwin_store = omnetpp::check_and_cast<DtwinStore*>(this->getParentModule()->getSubmodule("dtwinStore"));
    dtwin_store->updateObjectTypeAndAddress(subscriber_name, subscriber_type, subscriber_address);

    // Edit existing entry if the node already exists and if the time stamp is more recent, else create a new entry
    Subscription subscriber {
        subscriber_name,
        subscriber_address,
        std::string{msg->getRoi_tag()},
        std::string{msg->getQos_tag()},
        msg->getTimestamp()
    };

    auto it = std::find_if(std::begin(_subscriber_record), std::end(_subscriber_record), [&subscriber_name] (Subscription const& val) {return val.name == subscriber_name;});
    if(it != std::end(_subscriber_record)) {
        if((*it).timestamp < subscriber.timestamp)
            (*it) = subscriber;
    }
    else
        _subscriber_record.push_back(subscriber);

    EV_INFO << current_time <<" - SubscriberStore::updateSubscriberList(): " << "Subscriber record " << std::endl;
    for(auto sub:_subscriber_record)
        EV_INFO << sub.name << ", " << sub.id << std::endl;
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

std::vector<Subscription> SubscriberStore::readSubscriptions()
{
    return _subscriber_record;
}

} // namespace convoy_architecture
