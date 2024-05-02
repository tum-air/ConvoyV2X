
#include "HighwayScenario.h"

namespace convoy_architecture {

Define_Module(HighwayScenario);

HighwayScenario::~HighwayScenario()
{
    cancelAndDelete(_event);
    std::for_each(_rsus.begin(), _rsus.end(), [] (omnetpp::cModule *rsu) {
        if(rsu->isModule())
            rsu->deleteModule();
    });
    std::for_each(_roads.begin(), _roads.end(), [this] (omnetpp::cRectangleFigure *road) {
        getParentModule()->getCanvas()->removeFigure(road);
        delete road;
    });
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

    _event = new omnetpp::cMessage("setupRSU");
    scheduleAt(omnetpp::simTime(), _event);
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
    if(msg->isSelfMessage()) {
        if(std::string{msg->getName()} == std::string{"setupRSU"})
        {
            setupRSU(ConvoyDirection::TOP);
            setupRSU(ConvoyDirection::DOWN);
            if((++_counter) < par("n_stations").intValue())
                scheduleAfter(par("station_init_time_gap").doubleValue(), _event);
        }
    }
}

void HighwayScenario::setupRSU(ConvoyDirection direction) {
    // Work out location
    double x{0.0}, y{0.0};
    switch(direction)
    {
    case ConvoyDirection::TOP:
        x = par("station_top_start_x").doubleValue() + (par("inter_station_distance").doubleValue() * _counter);
        y = par("station_top_start_y").doubleValue();
        break;
    case ConvoyDirection::DOWN:
        x = par("station_bot_start_x").doubleValue() + (par("inter_station_distance").doubleValue() * _counter);
        y = par("station_bot_start_y").doubleValue();
        break;
    default:
        break;
    }

    // Setup station
    createInitRSU(_counter, (x + par("station_lx").doubleValue()/2), (y + par("station_ly").doubleValue()/2), direction);
}

void HighwayScenario::createInitRSU(int station_id, double x, double y, ConvoyDirection direction) {

    // Part I: create and initialize rsu
    // Instantiate and setup sensor station
    // Create submodule vector first, if it doesn't already exist
    std::string module_name = std::string("d") + std::to_string((int)direction) + std::string("rsu");
    if(!getParentModule()->hasSubmoduleVector(module_name.c_str()))
        getParentModule()->addSubmoduleVector(module_name.c_str(), par("n_stations").intValue());

    // Create the sensor station module
    omnetpp::cModule *module = omnetpp::cModuleType::get("convoy_architecture.nodes.BaseNode")->create(module_name.c_str(), getParentModule(), station_id);

    // Set position
    omnetpp::cDisplayString display_string = module->getDisplayString();
    display_string.setTagArg("p", 0, x);
    display_string.setTagArg("p", 1, y);
    module->setDisplayString(display_string.str());
    module->setDisplayName(nullptr);

    // Finalize module
    module->par("stationID") = (module_name + std::string("[") + std::to_string(station_id) + std::string("]")).c_str();
    omnetpp::cModule *backend_interface;
    switch(direction)
    {
    case ConvoyDirection::DOWN:
        module->par("detectorFovTag") = "down";
        module->par("detectorFovLimitAngleMin") = 0.00;
        module->par("detectorFovLimitAngleMax") = 3.13;
        backend_interface = getParentModule()->getSubmodule("backendInterfaceTop", station_id);
        break;
    default:
        module->par("detectorFovTag") = "top";
        module->par("detectorFovLimitAngleMin") = 3.14;
        module->par("detectorFovLimitAngleMax") = 6.27;
        backend_interface = getParentModule()->getSubmodule("backendInterfaceBot", station_id);
        break;
    }
    module->par("stationType") = (int) StationType::RSU;
    module->par("convoyDirection") = (int) direction;
    module->par("mobilityType") = "StationaryMobility";
    module->par("detectorStationType") = "rsu";
    module->par("detectorFovLimitRange") = par("station_fov_range").doubleValue();
    module->par("hasDtwinSub") = false;
    module->finalizeParameters();
    module->buildInside();
    module->scheduleStart(omnetpp::simTime());
    module->callInitialize();
    _rsus.push_back(module);

    // Part II: connect rsu to backend interface
    _rsus.back()->gate("outBkndPublisher")->connectTo(backend_interface->gate("inBkndPublisher"));
    _rsus.back()->gate("outBkndMemberReport")->connectTo(backend_interface->gate("inBkndMemberReport"));
    backend_interface->gate("outBkndMemberControl")->connectTo(_rsus.back()->gate("inBkndMemberControl"));
}
} // namespace convoy_architecture
