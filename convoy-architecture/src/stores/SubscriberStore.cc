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

#include "SubscriberStore.h"

namespace convoy_architecture {

Define_Module(SubscriberStore);

void SubscriberStore::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - SubscriberStore::initialize(): " << "Initializing subscriber store " << std::endl;

    double subscriber_max_age = par("subscriberMaxAge").doubleValue();
    _subscriber_expiry_check_event = new omnetpp::cMessage("subscriberExpiryCheck");
    scheduleAfter(par("subscriberCheckInterval").doubleValue(), _subscriber_expiry_check_event);

    EV_INFO << current_time <<" - SubscriberStore::initialize(): " << "Initialized subscriber store for station " << this->getParentModule()->getFullName() << std::endl;

    WATCH(_n_subscribers);
}

void SubscriberStore::handleMessage(omnetpp::cMessage *msg)
{
    // TODO - Generated method body
}

void SubscriberStore::appendNodeCCSFeedback(DtwinSub *msg) {
    // TODO
}

} // namespace convoy_architecture
