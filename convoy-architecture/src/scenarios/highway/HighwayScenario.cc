
#include "HighwayScenario.h"

namespace convoy_architecture {

Define_Module(HighwayScenario);

HighwayScenario::~HighwayScenario()
{
    cancelAndDelete(_event);
    for(auto road : _roads) {
        getParentModule()->getCanvas()->removeFigure(road);
        delete road;
    }
}

void HighwayScenario::initialize()
{
    double road_top_start_x {par("road_top_start_x").doubleValue()};
    double road_top_start_y {par("road_top_start_y").doubleValue()};
    double road_bot_start_x {par("road_bot_start_x").doubleValue()};
    double road_bot_start_y {par("road_bot_start_y").doubleValue()};
    double road_lx {par("road_lx").doubleValue()};
    double road_ly {par("road_ly").doubleValue()};

    // Draw the top and bottom road surfaces
    drawRoad(std::string{"road_top"}, road_top_start_x, road_top_start_y, road_lx, road_ly);
    drawRoad(std::string{"road_bot"}, road_bot_start_x, road_bot_start_y, road_lx, road_ly);

    /*
     * CONTINUE HERE
     */
}

void HighwayScenario::drawRoad(std::string name, double x, double y, double lx, double ly)
{
    _roads.emplace_back(new omnetpp::cRectangleFigure(name.c_str()));
    _roads.back()->setBounds(omnetpp::cFigure::Rectangle(x,y,lx,ly));
    _roads.back()->setLineColor(omnetpp::cFigure::Color("white"));
    _roads.back()->setLineWidth(2);
    _roads.back()->setFillColor(omnetpp::cFigure::Color("grey50"));
    _roads.back()->setFilled(true);
    _roads.back()->setZIndex(-1);
    getParentModule()->getCanvas()->addFigure(_roads.back());
}

void HighwayScenario::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

} // namespace convoy_architecture
