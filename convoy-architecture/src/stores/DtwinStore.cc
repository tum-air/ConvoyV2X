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

#include "DtwinStore.h"
#include "veins_inet/VeinsInetMobility.h"
#include <algorithm>

namespace convoy_architecture {

Define_Module(DtwinStore);

DtwinStore::DtwinStore()
{
    //
}

DtwinStore::~DtwinStore()
{
    cancelAndDelete(_object_expiry_check_event);
}

void DtwinStore::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - DtwinStore::initialize(): " << "Initializing dtwin-store application " << std::endl;

    _object_max_age = par("dTwinMaxAge").doubleValue();
    _object_expiry_check_interval = par("dTwinExpiryCheckInterval").doubleValue();
    _object_expiry_check_event = new omnetpp::cMessage("objectExpiryCheck");
    scheduleAt(omnetpp::simTime()+_object_expiry_check_interval, _object_expiry_check_event);

    EV_INFO << current_time <<" - DtwinStore::initialize(): " << "Initialized dtwin-store application for station " << this->getParentModule()->getFullName() << std::endl;

    WATCH(_n_objects);
}

void DtwinStore::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Check if the message is triggered internally
    if (msg->isSelfMessage())
    {
        // Check for object-expiry event
        if (msg == _object_expiry_check_event)
        {
            EV_INFO << current_time <<" - DtwinStore::handleMessage(): " << "Object-expiry check triggered " << std::endl;
            this->checkObjectExpiry();
            scheduleAt(current_time + _object_expiry_check_interval, _object_expiry_check_event);
        }
    }
    else if (strcmp(msg->getArrivalGate()->getName(), "in") == 0)
    {
        EV_INFO << current_time <<" - DtwinStore::handleMessage(): " << "External message received through input gate " << std::endl;
        ObjectList *tracked_objects = check_and_cast<ObjectList *>(msg);
        this->updateObjects(tracked_objects);
        delete tracked_objects;
    }

}

void DtwinStore::checkObjectExpiry()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    uint64_t current_timestamp = (uint64_t) current_time.raw();
    uint64_t duration_max_age = (uint64_t) _object_max_age.raw();

    // Iterate through each object
    if(_n_objects > 0)
    {
        for (auto it = _object_timestamp.cbegin(); it != _object_timestamp.cend();)
        {
            if((current_timestamp - it->second) >= duration_max_age)
            {
                EV_INFO << current_time <<" - DtwinStore::checkObjectExpiry(): max age threshold reached for object " << it->first << ", removing from store" << std::endl;
                _object_type.erase(it->first);
                _object_position_x.erase(it->first);
                _object_position_y.erase(it->first);
                _object_position_z.erase(it->first);
                _object_heading.erase(it->first);
                it = _object_timestamp.erase(it);
            }
            else
                ++it;
        }
        _n_objects = _object_timestamp.size();
    }
}

void DtwinStore::updateObjects(ObjectList *tracked_objects)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    uint64_t current_timestamp = (uint64_t) current_time.raw();
    uint64_t duration_max_age = (uint64_t) _object_max_age.raw();

    uint64_t obj_timestamp = tracked_objects->getTimestamp();
    int n_objects = tracked_objects->getN_objects();

    // Update the store only if it isn't too late
    if((current_timestamp - obj_timestamp) < duration_max_age)
    {
        for(int obj_index=0; obj_index<tracked_objects->getN_objects(); obj_index++)
        {
            // Update or insert the object information if it is either younger than the existing entry or if its non-existent
            std::string object_id = std::string(tracked_objects->getObject_id(obj_index));
            bool update_enabled = true;
            if(_object_id.find(object_id) != _object_id.end())
            {
                if(obj_timestamp < _object_timestamp[object_id])
                    update_enabled = false;
            }

            if(update_enabled)
            {
                _object_id.insert(object_id);
                if(tracked_objects->getObject_address(obj_index) >= 0)
                {
                    _object_address[object_id] = tracked_objects->getObject_address(obj_index);
                    _object_type[object_id] = tracked_objects->getObject_type(obj_index);
                }
                _object_timestamp[object_id] = obj_timestamp;
                _object_position_x[object_id] = tracked_objects->getObject_position_x(obj_index);
                _object_position_y[object_id] = tracked_objects->getObject_position_y(obj_index);
                _object_position_z[object_id] = tracked_objects->getObject_position_z(obj_index);
                _object_heading[object_id] = tracked_objects->getObject_heading(obj_index);
                EV_INFO << current_time <<" - DtwinStore::updateObjects(): " << "object entry updated for " << object_id << ", creation time: " << obj_timestamp << std::endl;
            }
        }
    }

    _n_objects = _object_id.size();
    EV_INFO << current_time << " - DtwinStore::updateObjects(): " << "Store update cycle completed. No of objects: " << _n_objects << std::endl;
}

ObjectList* DtwinStore::readFromStore()
{
    ObjectList* dtwin_message = new ObjectList();

    // Locate self
    omnetpp::cModule* station_module = this->getParentModule();
    inet::IMobility* station_mobility_module = check_and_cast<inet::IMobility*>(station_module->getSubmodule("mobility"));
    inet::Coord station_position_module = station_mobility_module->getCurrentPosition();

    // Fill up common information
    dtwin_message->setTimestamp((uint64_t) omnetpp::simTime().raw());
    dtwin_message->setStation_id(station_module->getFullName());
    dtwin_message->setStation_type(station_module->par("detectorStationType"));
    dtwin_message->setStation_position_x(station_position_module.x);
    dtwin_message->setStation_position_y(station_position_module.y);
    dtwin_message->setN_objects(_n_objects);

    // Fill in object details
    for (auto object_id:_object_id)
    {
        dtwin_message->appendObject_id(object_id.c_str());
        dtwin_message->appendObject_type(_object_type[object_id].c_str());
        dtwin_message->appendObject_address(_object_address[object_id]);
        dtwin_message->appendObject_position_x(_object_position_x[object_id]);
        dtwin_message->appendObject_position_y(_object_position_y[object_id]);
        dtwin_message->appendObject_position_z(_object_position_z[object_id]);
        dtwin_message->appendObject_heading(_object_heading[object_id]);
        dtwin_message->appendObject_timestamp(_object_timestamp[object_id]);
    }
    return(dtwin_message);
}


std::set<std::string> DtwinStore::readIDs()
{
    return(_object_id);
}

std::string DtwinStore::readType(std::string object_id)
{
    return(_object_type[object_id]);
}

double DtwinStore::readPositionX(std::string object_id)
{
    return(_object_position_x[object_id]);
}

double DtwinStore::readPositionY(std::string object_id)
{
    return(_object_position_y[object_id]);
}

double DtwinStore::readPositionZ(std::string object_id)
{
    return(_object_position_z[object_id]);
}

double DtwinStore::readHeading(std::string object_id)
{
    return(_object_heading[object_id]);
}

uint64_t DtwinStore::readTimestamp(std::string object_id)
{
    return(_object_timestamp[object_id]);
}

void DtwinStore::updateObjectTypeAndAddress(std::string object_id, std::string object_type, int object_address)
{
    if(_object_id.find(object_id) != _object_id.end())
    {
        _object_type[object_id] = object_type;
        _object_address[object_id] = object_address;
    }
}
} // namespace convoy_architecture
