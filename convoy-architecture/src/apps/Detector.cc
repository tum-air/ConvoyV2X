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

#include "Detector.h"
#include "veins_inet/VeinsInetMobility.h"
#include <algorithm>

namespace convoy_architecture {

Define_Module(Detector);

Detector::Detector()
{
    _start_event = nullptr;
    _update_event = nullptr;
    _detected_objects = nullptr;
}

Detector::~Detector()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
    if(_detected_objects != nullptr)
        delete _detected_objects;
}

void Detector::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - Detector::initialize(): " << "Initializing detector application " << std::endl;

    _fov_tag = par("fovTag").stdstringValue();
    _fov_limit_range = par("fovLimitRange").doubleValue();
    _fov_limit_angle_min = par("fovLimitAngleMin").doubleValue();
    _fov_limit_angle_max = par("fovLimitAngleMax").doubleValue();
    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");
    _detected_objects = new ObjectList();

    std::stringstream detected_classes(par("detectedClasses").stdstringValue());
    while(detected_classes.good())
    {
        std::string detected_class;
        std::getline(detected_classes, detected_class, ',');
        _detected_object_classes.push_back(detected_class);
        EV_INFO << current_time <<" - Detector::initialize(): " << "Tracking " << _detected_object_classes.back() << std::endl;
    }

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - Detector::initialize(): " << "Scheduled detector application start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - Detector::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
    }

    EV_INFO << current_time <<" - Detector::initialize(): " << "Initialized detector application for station " << this->getParentModule()->getFullName() << std::endl;
}

void Detector::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - Detector::handleMessage(): " << "Starting detector application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - Detector::handleMessage(): " << "Updating detector estimates" << std::endl;
            this->updateDetections();
            EV_INFO << current_time <<" - Detector::handleMessage(): " << "Publishing detector estimates" << std::endl;
            this->publishDetectedObjects();
        }
        scheduleAfter(_update_rate, _update_event);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - Detector::handleMessage(): " << "Stopping detector application" << std::endl;
        cancelEvent(_update_event);
    }
}

void Detector::updateDetections()
{
    // This function can be extended to alter the detection sequences
    this->gatherDetectedObjects();
}

void Detector::gatherDetectedObjects()
{
    // This function can be extended to include sensor models and detection algorithms
    // It can furthermore extended to play back from recorded digital twin data sets
    // In this version, traffic objects are directly gathered from the simulation environment

    // Clear out detections from last time stamp
    _detected_objects->setN_objects(0);
    _detected_objects->setObject_idArraySize(0);
    _detected_objects->setObject_typeArraySize(0);
    _detected_objects->setObject_addressArraySize(0);
    _detected_objects->setObject_timestampArraySize(0);
    _detected_objects->setObject_position_xArraySize(0);
    _detected_objects->setObject_position_yArraySize(0);
    _detected_objects->setObject_position_zArraySize(0);
    _detected_objects->setObject_headingArraySize(0);

    // Locate self
    omnetpp::cModule* station_module = this->getParentModule();
    inet::IMobility* station_mobility_module = check_and_cast<inet::IMobility*>(station_module->getSubmodule("mobility"));
    inet::Coord station_position_module = station_mobility_module->getCurrentPosition();

    // Fill up common information
    _detected_objects->setTimestamp((uint64_t) omnetpp::simTime().raw());
    _detected_objects->setStation_id(station_module->getFullName());
    _detected_objects->setStation_type(station_module->par("detectorStationType"));
    _detected_objects->setStation_position_x(station_position_module.x);
    _detected_objects->setStation_position_y(station_position_module.y);

    // Iterate through all traffic objects present in the simulation
    omnetpp::cModule* system_module = getSimulation()->getSystemModule();

    for (std::string detected_object_class : _detected_object_classes)
    {
        if(system_module->hasSubmoduleVector(detected_object_class.c_str()))
        {
            int n_objects = system_module->getSubmoduleVectorSize(detected_object_class.c_str());
            EV_INFO << omnetpp::simTime() <<" - Detector::gatherDetectedObjects(): " << n_objects << " x " << detected_object_class << " in simulation" << std::endl;
            for(int i=0; i<n_objects; i++)
            {
                omnetpp::cModule* object_module = system_module->getSubmodule(detected_object_class.c_str(), i);
                if(object_module != nullptr)
                {
                    veins::VeinsInetMobility* object_mobility_module = check_and_cast<veins::VeinsInetMobility*>(object_module->getSubmodule("mobility"));
                    inet::Coord object_position_module = object_mobility_module->getCurrentPosition();

                    // Check for coverage range
                    double object_distance_to_station = std::abs(object_position_module.distance(station_position_module));
                    inet::Quaternion object_quaternion = object_mobility_module->getCurrentAngularPosition();
                    double object_rotation_angle = object_quaternion.getRotationAngle();
                    bool within_distance_range = object_distance_to_station <= _fov_limit_range;
                    bool within_angular_range = (object_rotation_angle >= _fov_limit_angle_min) && (object_rotation_angle < _fov_limit_angle_max);
                    if(within_distance_range && within_angular_range)
                    {
                        std::vector<double> object_position {object_position_module.x, object_position_module.y};
                        _detected_objects->appendObject_id(object_module->getFullName());
                        _detected_objects->appendObject_type(object_module->par("detectorStationType"));
                        _detected_objects->appendObject_position_x(object_position_module.x);
                        _detected_objects->appendObject_position_y(object_position_module.y);
                        _detected_objects->appendObject_position_z(0);
                        _detected_objects->appendObject_heading(object_rotation_angle);
                        _detected_objects->appendObject_timestamp((uint64_t) omnetpp::simTime().raw());
                        _detected_objects->appendObject_address(-1);
                        _detected_objects->setN_objects(_detected_objects->getN_objects() + 1);
                    }
                }
            }
        }
    }

    EV_INFO << omnetpp::simTime() <<" - Detector::gatherDetectedObjects(): " << "Detected " << _detected_objects->getN_objects() << " objects" << std::endl;
}

void Detector::publishDetectedObjects()
{
    std::string message_name =  std::string("dtwin_") +
                                std::to_string(_detected_objects->getTimestamp()) + std::string("_") +
                                std::string(_detected_objects->getStation_id()) +
                                std::string("_fov_") + _fov_tag;
    ObjectList* dtwin_message = _detected_objects->dup();
    dtwin_message->setName(message_name.c_str());
    this->send(dtwin_message, "detectionsEgo");
    EV_INFO << omnetpp::simTime() <<" - Detector::publishDetectedObjects(): " << "Published list of detected objects" << std::endl;
}

} // namespace convoy_architecture
