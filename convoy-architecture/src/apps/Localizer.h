
#ifndef __CONVOY_ARCHITECTURE_LOCALIZER_H_
#define __CONVOY_ARCHITECTURE_LOCALIZER_H_

#include <omnetpp.h>
#include <vector>

namespace convoy_architecture {

class Localizer : public omnetpp::cSimpleModule
{
  public:
    enum LocalizationState {STATE_INIT, STATE_LOCALIZING, STATE_LOCALIZED};
    enum LocalizationStrategy {STRATEGY_VISUAL_CUES}; // can be extended
    ~Localizer();
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
    omnetpp::cMessage* _start_event{nullptr};
    omnetpp::cMessage* _update_event{nullptr};
    double _min_duration_localization;
    double _max_age_localization;
    LocalizationStrategy _localization_strategy;
    LocalizationState _state;
    std::string _ego_id {""};
    std::string _ego_type {""};
    uint64_t _ego_timestamp {0};
    double _ego_position_x {0.0};
    double _ego_position_y {0.0};
    double _ego_position_z {0.0};
    double _ego_heading {0.0};
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
