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

#include "Tracker.h"

namespace convoy_architecture {

Define_Module(Tracker);

Tracker::Tracker()
{
    _tracked_objects = nullptr;
}

Tracker::~Tracker()
{
    if(_tracked_objects != nullptr)
            delete _tracked_objects;
}

void Tracker::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - Tracker::initialize(): " << "Initializing tracker application " << std::endl;

    _tracked_objects = new ObjectList();
    EV_INFO << current_time <<" - Tracker::initialize(): " << "Initialized tracker application for station " << this->getParentModule()->getFullName() << std::endl;
}

void Tracker::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    if (!msg->isSelfMessage())
    {
        EV_INFO << current_time <<" - Tracker::handleMessage(): " << "Updating tracker estimates" << std::endl;
        ObjectList *detected_objects = omnetpp::check_and_cast<ObjectList *>(msg);
        this->updateTrackedObjects(detected_objects);
        EV_INFO << current_time <<" - Tracker::handleMessage(): " << "Publishing tracker estimates" << std::endl;
        this->publishTrackedObjects();
    }
}

void Tracker::updateTrackedObjects(ObjectList *detected_objects)
{
    // This function can be extended to include appropriate tracking algorithms
    // In this version, the detected objects are identified with their module names from the simulation
    // This is already done in the Detector module
    // The detected objects are therefore merely copied as tracked objects
    if(_tracked_objects != nullptr)
        delete _tracked_objects;
    _tracked_objects = detected_objects;
    EV_INFO << omnetpp::simTime() <<" - Tracker::updateTrackedObjects(): " << "Updated list of tracked objects" << std::endl;
}

void Tracker::publishTrackedObjects()
{
    ObjectList *dtwin_message = _tracked_objects->dup();
    this->send(dtwin_message, "dtwinCollective");
    EV_INFO << omnetpp::simTime() <<" - Tracker::publishTrackedObjects(): " << "Published list of tracked objects" << std::endl;
}

} // namespace convoy_architecture
