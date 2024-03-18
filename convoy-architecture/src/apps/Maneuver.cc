
#include "Maneuver.h"

namespace convoy_architecture {

Define_Module(Maneuver);

Maneuver::Maneuver()
{
    _start_event = nullptr;
    _update_event = nullptr;
}

Maneuver::~Maneuver()
{
    cancelAndDelete(_start_event);
    cancelAndDelete(_update_event);
}

void Maneuver::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - Maneuver::initialize(): " << "Initializing coop. maneuvering application " << std::endl;

    _ego_id = par("egoID").stdstringValue();
    _roi_tag = par("roiTag").stdstringValue();
    _qos_tag = par("qosTag").stdstringValue();
    _start_time = par("startTime").doubleValue();
    _stop_time = par("stopTime").doubleValue();
    _update_rate = par("updateRate").doubleValue();
    if(_roi_tag == "neighborhood")
        _limit_roi_distance = par("limitROINeighborhood").doubleValue();
    else if(_roi_tag == "horizon")
        _limit_roi_distance = par("limitROIHorizon").doubleValue();
    else
        _limit_roi_distance = par("limitROIDefault").doubleValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");

    _dtwin_store = check_and_cast<DtwinStore*>(this->getParentModule()->getSubmodule("dtwinStore"));
    _localizer_app = check_and_cast<Localizer*>(this->getParentModule()->getSubmodule("appLocalizer"));

    if(_stop_time > _start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < _start_time)? _start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - Maneuver::initialize(): " << "Scheduled coop. maneuvering application start for time " << trigger_time << "s" << std::endl;
        EV_INFO << current_time <<" - Maneuver::initialize(): " << "Update rate set at " << _update_rate << "s" << std::endl;
    }

    EV_INFO << current_time <<" - Maneuver::initialize(): " << "Initialized coop. maneuvering application for station " << this->getParentModule()->getFullName() << std::endl;
}

void Maneuver::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - Maneuver::handleMessage(): " << "Starting coop. maneuvering application" << std::endl;
        }
        else if (msg == _update_event)
        {
            // Proceed only if the localizer is ready
            if(_localizer_app->isLocalized())
            {
                this->findCoopManeuverPartner();
                this->sendCoopManeuverMessage(this->createCoopManeuverMessage());
            }
            else
                EV_INFO << current_time <<" - Maneuver::handleMessage(): " << "localizer not yet ready, ignoring event " << std::endl;
        }
        scheduleAfter(_update_rate, _update_event);
    }
    else if (strcmp(msg->getArrivalGate()->getName(), "in") == 0)
    {
        EV_INFO << current_time <<" - Maneuver::handleMessage(): " << "External message received through input gate " << std::endl;
        CoopManeuver *coop_maneuver_message = check_and_cast<CoopManeuver *>(msg);
        this->receiveCoopManeuverMessage(coop_maneuver_message);
    }

    if ((current_time >= _stop_time) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - Maneuver::handleMessage(): " << "Stopping coop. maneuvering application" << std::endl;
        cancelEvent(_update_event);
    }
}

void Maneuver::findCoopManeuverPartner()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    _partner_id.clear();
    _partner_address.clear();

    // Update the list of partners based on dtwin info
    ObjectList *dtwin_list = _dtwin_store->readFromStore();

    // Locate self - read from localizer app
    _ego_id = _localizer_app->readEgoID();
    std::string ego_type = _localizer_app->readEgoType();
    double ego_position_x = _localizer_app->readEgoPositionX();
    double ego_position_y = _localizer_app->readEgoPositionY();
    double ego_position_z = _localizer_app->readEgoPositionZ();
    double ego_heading = _localizer_app->readEgoHeading();

    // Figure out partners by iterating through dtwin list
    for(int dtwin_index=0; dtwin_index<dtwin_list->getN_objects(); dtwin_index++)
    {
        std::string candidate_id = dtwin_list->getObject_id(dtwin_index);
        if (candidate_id != _ego_id)
        {
            // Read candidate position and heading
            std::string candidate_type = dtwin_list->getObject_type(dtwin_index);
            int candidate_address = dtwin_list->getObject_address(dtwin_index);
            double candidate_position_x = dtwin_list->getObject_position_x(dtwin_index);
            double candidate_position_y = dtwin_list->getObject_position_y(dtwin_index);
            double candidate_position_z = dtwin_list->getObject_position_z(dtwin_index);
            double candidate_heading = dtwin_list->getObject_heading(dtwin_index);
            double distance_x = std::abs(ego_position_x-candidate_position_x);
            double distance_y = std::abs(ego_position_y-candidate_position_y);
            double distance_z = std::abs(ego_position_z-candidate_position_z);
            double distance = sqrt(pow(distance_x, 2) + pow(distance_y, 2) + pow(distance_z, 2));

            // Add candidate to partner list if
            // the candidate is driving in the same direction - heading check
            // the candidate is located within the region of interest
            // the candidate is V2V capable - assuming to be encoded into object id
            // for now reading from stationType parameter
            bool same_driving_direction = std::abs(ego_heading - candidate_heading) < 1.57;
            bool within_neighborhood =  distance <= _limit_roi_distance;
            bool v2v_capable = (candidate_type == std::string("v2v"));
            if(same_driving_direction && within_neighborhood && v2v_capable)
            {
                _partner_id.push_back(candidate_id);
                _partner_address.push_back(candidate_address);
                EV_INFO << current_time << " - Maneuver::findCoopManeuverPartner(): " << "Found partner for coop. maneuver: " << candidate_id << " - " << candidate_address << std::endl;
            }
        }
    }
}

std::vector<CoopManeuver*> Maneuver::createCoopManeuverMessage()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Prepare partner specific coop. maneuver message
    std::vector<CoopManeuver*> coop_maneuver_message_list;

    int n_partners = _partner_id.size();
    if(n_partners > 0)
    {
        for (int i=0; i<n_partners; i++)
        {
            coop_maneuver_message_list.push_back(new CoopManeuver());
            coop_maneuver_message_list.back()->setTimestamp((uint64_t)current_time.raw());
            coop_maneuver_message_list.back()->setEgo_id(_ego_id.c_str());
            coop_maneuver_message_list.back()->setPartner_id(_partner_id.at(i).c_str());
            coop_maneuver_message_list.back()->setPartner_address(_partner_address.at(i));
        }
    }
    else
        EV_INFO << current_time << " - Maneuver::createCoopManeuverMessage(): " << "No partners available for coop. maneuver" << std::endl;

    return(coop_maneuver_message_list);
}

void Maneuver::sendCoopManeuverMessage(std::vector<CoopManeuver*> coop_maneuver_message_list)
{
    for (CoopManeuver* coop_maneuver_message : coop_maneuver_message_list)
        this->send(coop_maneuver_message, "out");
}

void Maneuver::receiveCoopManeuverMessage(CoopManeuver* coop_maneuver_message)
{
    // Cooperative maneuvering function can be implemented here
    // Deleting the received message for now
    delete coop_maneuver_message;
}

void Maneuver::updateEgoID(std::string ego_id)
{
    _ego_id = ego_id;
}

void Maneuver::updateROI(std::string roi_tag)
{
    _roi_tag = roi_tag;
}

void Maneuver::updateQOS(std::string qos_tag)
{
    _qos_tag = qos_tag;
}

} // namespace convoy_architecture
