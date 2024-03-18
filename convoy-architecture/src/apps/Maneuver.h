
#ifndef __CONVOY_ARCHITECTURE_MANEUVER_H_
#define __CONVOY_ARCHITECTURE_MANEUVER_H_

#include <omnetpp.h>
#include <vector>
#include "messages/CoopManeuver_m.h"
#include "stores/DtwinStore.h"
#include "apps/Localizer.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class Maneuver : public omnetpp::cSimpleModule
{
private:
    std::string  _ego_id;
    std::string  _roi_tag;
    std::string  _qos_tag;
    omnetpp::simtime_t _start_time;
    omnetpp::simtime_t _stop_time;
    omnetpp::simtime_t _update_rate;
    omnetpp::cMessage* _start_event;
    omnetpp::cMessage* _update_event;
    std::vector<std::string> _partner_id;
    std::vector<int> _partner_address;
    double _limit_roi_distance;
    DtwinStore* _dtwin_store;
    Localizer* _localizer_app;
    void findCoopManeuverPartner();
    std::vector<CoopManeuver*> createCoopManeuverMessage();
    void sendCoopManeuverMessage(std::vector<CoopManeuver*> coop_maneuver_message_list);
    void receiveCoopManeuverMessage(CoopManeuver* coop_maneuver_message);

protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
public:
    ~Maneuver();
    Maneuver();
    void updateROI(std::string roi_tag);
    void updateQOS(std::string qos_tag);
    void updateEgoID(std::string ego_id);
};

} // namespace convoy_architecture

#endif
