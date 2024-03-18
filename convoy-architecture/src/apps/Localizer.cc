
#include "Localizer.h"
#include "messages/ObjectList_m.h"
#include "nodes/BaseNode.h"

namespace convoy_architecture {

Define_Module(Localizer);

Localizer::Localizer()
{
    _start_event = nullptr;
    _update_event = nullptr;
}

Localizer::~Localizer()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
}

void Localizer::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - Localizer::initialize(): " << "Initializing localization application " << std::endl;

    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();
    _min_duration_localization = par("minDurationLocalization").doubleValue();
    _max_age_localization = par("maxAgeLocalization").doubleValue();
    std::string localization_strategy = par("strategyLocalization").stdstringValue();
    if(localization_strategy == "VISUAL_CUE")
        _localization_strategy = STRATEGY_VISUAL_CUES;
    else
    {
        // can be extended for other strategies
    }

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");

    _dtwin_store = check_and_cast<DtwinStore*>(this->getParentModule()->getSubmodule("dtwinStore"));

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - Localizer::initialize(): " << "Scheduled localization application start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - Localizer::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
    }

    // Set the initial localization state
    _state = STATE_INIT;
    EV_INFO << current_time << " - Localizer::initialize(): " << "Initialized localization application for station " << this->getParentModule()->getFullName() << std::endl;
    WATCH(_state);
    WATCH(_ego_id);
    WATCH(_ego_type);
    WATCH(_ego_position_x);
    WATCH(_ego_position_y);
    WATCH(_ego_heading);
    WATCH(_ego_timestamp);
}

void Localizer::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - Localizer::handleMessage(): " << "Starting localization application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - Localizer::handleMessage(): " << "Updating ego data" << std::endl;
            this->egoStateUpdate();
        }
        EV_INFO << current_time <<" - Localizer::handleMessage(): " << "Updating localization state" << std::endl;
        this->localizationStateUpdate();
        scheduleAfter(_update_rate, _update_event);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - Localizer::handleMessage(): " << "Stopping localization application" << std::endl;
        cancelEvent(_update_event);
    }
}

void Localizer::egoStateUpdate()
{
    switch (_state)
    {
    case STATE_INIT:
        // No ego state update
        break;
    case STATE_LOCALIZING:
        // No ego state update
        break;
    case STATE_LOCALIZED:
        readCurrentLocation();
        break;
    default:
        break;
    }
}

void Localizer::localizationStateUpdate()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    omnetpp::simtime_t localization_duration = (current_time - _init_time);
    int64_t object_age = current_time.raw() - (int64_t)_ego_timestamp;
    switch (_state)
    {
    case STATE_INIT:
        _init_time = current_time;
        _state = STATE_LOCALIZING;
        break;
    case STATE_LOCALIZING:
        this->executeLocalizationStrategy();
        if((localization_duration.dbl() >= _min_duration_localization) && (_ego_id != "NULL"))
        {
            readCurrentLocation();
            _state = STATE_LOCALIZED;
        }
        break;
    case STATE_LOCALIZED:
        // Check for maximum localization age
        // For cases where digital twin is not available due to track losses
        // Or is not transferred due to network packet losses
        if (omnetpp::simtime_t::fromRaw(object_age).dbl() > _max_age_localization)
            _state = STATE_INIT;
        break;
    default:
        break;
    }
}

void Localizer::executeLocalizationStrategy()
{
    // Function to implement localization strategy
    switch (_localization_strategy)
    {
    case STRATEGY_VISUAL_CUES:
        // Assuming that the visual cues are encode into object id string information
        _ego_id = localizationStrategyVisualCues(_dtwin_store->readIDs());
        break;
    default:
        _ego_id = std::string("NULL");
        break;
    }
}

std::string Localizer::localizationStrategyVisualCues(std::set<std::string> object_ids)
{
    std::string ego_id = std::string("NULL");

    // Assuming that the visual cues are encoded into object id string information
    // Visual cue matching can be implemented here
    // Temporarily using the name of the parent as the object id
    ego_id = this->getParentModule()->getFullName();
    return(ego_id);
}

bool Localizer::isLocalized()
{
    return(_state == STATE_LOCALIZED);
}

std::string Localizer::readEgoID()
{
    return(_ego_id);
}

std::string Localizer::readEgoType()
{
    return(_ego_type);
}

double Localizer::readEgoPositionX()
{
    double ego_position = _ego_position_x;
    return(ego_position);
}

double Localizer::readEgoPositionY()
{
    double ego_position = _ego_position_y;
    return(ego_position);
}

double Localizer::readEgoPositionZ()
{
    double ego_position = _ego_position_z;
    return(ego_position);
}

double Localizer::readEgoHeading()
{
    return(_ego_heading);
}

void Localizer::readCurrentLocation()
{
    std::string parent_type = par("stationType").stdstringValue();
    if(parent_type == std::string("vehicle"))
    {
        _ego_type = _dtwin_store->readType(_ego_id);
        _ego_position_x = _dtwin_store->readPositionX(_ego_id);
        _ego_position_y = _dtwin_store->readPositionY(_ego_id);
        _ego_position_z = _dtwin_store->readPositionZ(_ego_id);
        _ego_heading = _dtwin_store->readHeading(_ego_id);
        _ego_timestamp = _dtwin_store->readTimestamp(_ego_id);
    }
    else
    {
        BaseNode* station_module = check_and_cast<BaseNode*>(getParentModule());
        inet::Coord station_location = station_module->getCurrentLocation();
        _ego_type = std::string("rsu");
        _ego_position_x = station_location.x;
        _ego_position_y = station_location.y;
        _ego_position_z = station_location.z;
        _ego_heading = 0;
        _ego_timestamp = (uint64_t) omnetpp::simTime().raw();
    }

}
} // namespace convoy_architecture
