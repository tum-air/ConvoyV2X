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

#include "DtwinPublisher.h"

namespace convoy_architecture {

Define_Module(DtwinPublisher);

DtwinPublisher::DtwinPublisher()
{
    _start_event = nullptr;
    _update_event = nullptr;
}

DtwinPublisher::~DtwinPublisher()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
}

void DtwinPublisher::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - DtwinPublisher::initialize(): " << "Initializing dtwin publisher application " << std::endl;

    _publisher_id = par("publisherID").stdstringValue();
    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");
    _dtwin_store = check_and_cast<DtwinStore*>(this->getParentModule()->getSubmodule("dtwinStore"));

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - DtwinPublisher::initialize(): " << "Scheduled dtwin publisher start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - DtwinPublisher::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
    }

    EV_INFO << current_time <<" - DtwinPublisher::initialize(): " << "Initialized dtwin publisher application for station " << this->getParentModule()->getFullName() << std::endl;
}

void DtwinPublisher::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - DtwinPublisher::handleMessage(): " << "Starting dtwin publisher application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - DtwinPublisher::handleMessage(): " << "Reading from dtwin store and publishing" << std::endl;
            this->sendDtwinMessage(this->readDtwin());
        }
        scheduleAfter(_update_rate, _update_event);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - DtwinPublisher::handleMessage(): " << "Stopping dtwin publisher application" << std::endl;
        cancelEvent(_update_event);
    }
}

ObjectList* DtwinPublisher::readDtwin()
{
    return(_dtwin_store->readFromStore());
}

void DtwinPublisher::sendDtwinMessage(ObjectList *dtwin_message)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Send out the message only if tracked objects are available
    int n_objects = dtwin_message->getN_objects();
    if(n_objects > 0)
    {
        this->send(dtwin_message->dup(), "out_bknd");
        this->send(dtwin_message, "out");
        EV_INFO << current_time <<" - DtwinPublisher::sendDtwinMessage(): " << "Sent out dtwin message with n_objects = " << n_objects << std::endl;
    }
    else
    {
        EV_INFO << current_time <<" - DtwinPublisher::sendDtwinMessage(): " << "No objects present in dtwin message, ignoring message" << std::endl;
        delete dtwin_message;
    }
}
} // namespace convoy_architecture
