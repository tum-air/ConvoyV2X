
#include "Publisher.h"

namespace convoy_architecture {

Define_Module(Publisher);

Publisher::Publisher()
{
    _start_event = nullptr;
    _update_event = nullptr;
}

Publisher::~Publisher()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
}

void Publisher::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - Publisher::initialize(): " << "Initializing dtwin publisher application " << std::endl;

    _publisher_id = par("publisherID").stdstringValue();
    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");
    _dtwin_store = omnetpp::check_and_cast<DtwinStore*>(this->getParentModule()->getSubmodule("dtwinStore"));

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - Publisher::initialize(): " << "Scheduled dtwin publisher start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - Publisher::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
    }

    EV_INFO << current_time <<" - Publisher::initialize(): " << "Initialized dtwin publisher application for station " << this->getParentModule()->getFullName() << std::endl;
}

void Publisher::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - Publisher::handleMessage(): " << "Starting dtwin publisher application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - Publisher::handleMessage(): " << "Reading from dtwin store and publishing" << std::endl;
            this->sendDtwinMessage(this->readDtwin());
        }
        scheduleAfter(_update_rate, _update_event);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - Publisher::handleMessage(): " << "Stopping dtwin publisher application" << std::endl;
        cancelEvent(_update_event);
    }
}

ObjectList* Publisher::readDtwin()
{
    return(_dtwin_store->readFromStore());
}

void Publisher::sendDtwinMessage(ObjectList *dtwin_message)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Send out the message only if tracked objects are available
    int n_objects = dtwin_message->getN_objects();
    if(n_objects > 0)
    {
        this->send(dtwin_message, "out");
        EV_INFO << current_time <<" - Publisher::sendDtwinMessage(): " << "Sent out dtwin message with n_objects = " << n_objects << std::endl;
    }
    else
    {
        EV_INFO << current_time <<" - Publisher::sendDtwinMessage(): " << "No objects present in dtwin message, ignoring message" << std::endl;
        delete dtwin_message;
    }
}

} // namespace convoy_architecture
