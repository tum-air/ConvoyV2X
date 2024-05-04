
#ifndef __CONVOY_ARCHITECTURE_TRACKER_H_
#define __CONVOY_ARCHITECTURE_TRACKER_H_

#include <omnetpp.h>
#include "messages/ObjectList_m.h"

namespace convoy_architecture {

class Tracker : public omnetpp::cSimpleModule
{
  private:
    ObjectList* _tracked_objects{nullptr};
    virtual void updateTrackedObjects(ObjectList *detected_objects);
    virtual void publishTrackedObjects();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

  public:
    ~Tracker();
};

} // namespace convoy_architecture

#endif
