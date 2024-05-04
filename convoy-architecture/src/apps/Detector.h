
#ifndef __CONVOY_ARCHITECTURE_DETECTOR_H_
#define __CONVOY_ARCHITECTURE_DETECTOR_H_

#include <omnetpp.h>
#include "messages/ObjectList_m.h"

namespace convoy_architecture {

class Detector : public omnetpp::cSimpleModule
{
  private:
    std::string  _fov_tag;
    double _fov_limit_range;
    double _fov_limit_angle_min;
    double _fov_limit_angle_max;
    omnetpp::simtime_t _start_time;
    omnetpp::simtime_t _stop_time;
    omnetpp::simtime_t _update_rate;
    omnetpp::cMessage* _start_event{nullptr};
    omnetpp::cMessage* _update_event{nullptr};
    ObjectList* _detected_objects{nullptr};
    std::vector<std::string> _detected_object_classes;
    void updateDetections();
    void gatherDetectedObjects();
    void publishDetectedObjects();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

  public:
    ~Detector();
};

} // namespace convoy_architecture

#endif
