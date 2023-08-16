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

#ifndef __CONVOY_ARCHITECTURE_LOCALIZER_H_
#define __CONVOY_ARCHITECTURE_LOCALIZER_H_

#include <omnetpp.h>
#include <vector>
#include "stores/DtwinStore.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class Localizer : public omnetpp::cSimpleModule
{
  public:
    enum LocalizationState {STATE_INIT, STATE_LOCALIZING, STATE_LOCALIZED};
    enum LocalizationStrategy {STRATEGY_VISUAL_CUES}; // can be extended
    ~Localizer();
    Localizer();
    bool isLocalized();
    std::string readEgoID();
    std::string readEgoType();
    double readEgoPositionX();
    double readEgoPositionY();
    double readEgoPositionZ();
    double readEgoHeading();
  private:
    omnetpp::simtime_t _start_time;
    omnetpp::simtime_t _init_time;
    omnetpp::simtime_t _stop_time;
    omnetpp::simtime_t _update_rate;
    omnetpp::cMessage* _start_event;
    omnetpp::cMessage* _update_event;
    double _min_duration_localization;
    double _max_age_localization;
    LocalizationStrategy _localization_strategy;
    LocalizationState _state;
    std::string _ego_id;
    std::string _ego_type;
    uint64_t _ego_timestamp;
    double _ego_position_x;
    double _ego_position_y;
    double _ego_position_z;
    double _ego_heading;
    DtwinStore* _dtwin_store;
    void egoStateUpdate();
    void localizationStateUpdate();
    std::string localizationStrategyVisualCues(std::set<std::string> object_ids);
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    virtual void executeLocalizationStrategy();
    void readCurrentLocation();
};

} // namespace convoy_architecture

#endif
