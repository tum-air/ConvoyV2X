
#ifndef __CONVOY_ARCHITECTURE_CLUSTERDEVICE_H_
#define __CONVOY_ARCHITECTURE_CLUSTERDEVICE_H_

#include <omnetpp.h>

namespace convoy_architecture {

/**
 * Common cluster device
 */
class ClusterDevice : public omnetpp::cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
};

} // namespace convoy_architecture

#endif
