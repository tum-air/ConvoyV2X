
#ifndef __CONVOY_ARCHITECTURE_HIGHWAYSCENARIO_H_
#define __CONVOY_ARCHITECTURE_HIGHWAYSCENARIO_H_

#include <omnetpp.h>
#include "common/defs.h"

namespace convoy_architecture {

class HighwayScenario : public cSimpleModule
{
  private:
    int _counter {0};
    omnetpp::cMessage* _event{nullptr};
    std::vector<omnetpp::cRectangleFigure *> _roads{};
    std::vector<omnetpp::cModule*> _rsus;
    void drawRoad(std::string name, double x, double y, double lx, double ly);
    void setupRSU(ConvoyDirection direction);
    void createInitRSU(int station_id, double x, double y, ConvoyDirection direction);

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  public:
      ~HighwayScenario();
};

} // namespace convoy_architecture

#endif
