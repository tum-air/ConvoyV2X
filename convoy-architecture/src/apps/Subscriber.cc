
#include "Subscriber.h"

namespace convoy_architecture {

Define_Module(Subscriber);

Subscriber::Subscriber()
{
    _start_event = nullptr;
    _update_event = nullptr;
}

Subscriber::~Subscriber()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
}

void Subscriber::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - Subscriber::initialize(): " << "Initializing dtwin subscriber application " << std::endl;

    _subscriber_id = par("subscriberID").stdstringValue();
    _roi_tag = par("roiTag").stdstringValue();
    _qos_tag = par("qosTag").stdstringValue();
    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();
    _subscriber_type = par("subscriberType").stdstringValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - Subscriber::initialize(): " << "Scheduled dtwin subscriber application start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - Subscriber::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
    }

    EV_INFO << current_time <<" - Subscriber::initialize(): " << "Initialized dtwin subscriber application for station " << this->getParentModule()->getFullName() << std::endl;
}

void Subscriber::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    updateSubscriberID(getParentModule()->getFullName());

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - Subscriber::handleMessage(): " << "Starting dtwin subscriber application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - Subscriber::handleMessage(): " << "Publishing dtwin subscription message" << std::endl;
            this->sendDtwinSubscriptionMessage();
        }
        scheduleAfter(_update_rate, _update_event);
    }
    else
    {
        EV_INFO << current_time <<" - Subscriber::handleMessage(): " << "Receiving dtwin message" << std::endl;
        ObjectList *dtwin_message = omnetpp::check_and_cast<ObjectList *>(msg);
        this->receiveDtwinMessage(dtwin_message);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - Subscriber::handleMessage(): " << "Stopping dtwin subscriber application" << std::endl;
        cancelEvent(_update_event);
    }
}

void Subscriber::sendDtwinSubscriptionMessage()
{
    DtwinSub *sub_message = new DtwinSub();
    sub_message->setTimestamp((uint64_t) omnetpp::simTime().raw());
    sub_message->setSubscriber_id(_subscriber_id.c_str());
    sub_message->setRoi_tag(_roi_tag.c_str());
    sub_message->setQos_tag(_qos_tag.c_str());
    sub_message->setSubscriber_type(_subscriber_type.c_str());
    this->send(sub_message, "out_sub");
}

void Subscriber::receiveDtwinMessage(ObjectList *dtwin_message)
{
    this->send(dtwin_message, "out_dtwin");
}

void Subscriber::updateSubscriberID(std::string subscriber_id)
{
    _subscriber_id = subscriber_id;
}

void Subscriber::updateROI(std::string roi_tag)
{
    _roi_tag = roi_tag;
}

void Subscriber::updateQOS(std::string qos_tag)
{
    _qos_tag = qos_tag;
}

} // namespace convoy_architecture
