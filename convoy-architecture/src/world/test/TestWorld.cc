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

#include "TestWorld.h"
#include "inet/common/InitStages.h"
#include "common/defs.h"

namespace convoy_architecture {

Define_Module(TestWorld);

TestWorld::TestWorld()
{
    _setup_trigger_top = nullptr;
    _setup_trigger_bot = nullptr;
    _heartbeat_trigger = nullptr;
}
TestWorld::~TestWorld()
{
    cancelAndDelete(_setup_trigger_top);
    cancelAndDelete(_setup_trigger_bot);
    cancelAndDelete(_heartbeat_trigger);
}

void TestWorld::initialize()
{
    _n_stations = par("n_stations").intValue();
    _road_top_start_x = par("road_top_start_x").doubleValue();
    _road_top_start_y = par("road_top_start_y").doubleValue();
    _road_bot_start_x = par("road_bot_start_x").doubleValue();
    _road_bot_start_y = par("road_bot_start_y").doubleValue();
    _road_lx = par("road_lx").doubleValue();
    _road_ly = par("road_ly").doubleValue();
    _station_top_start_x = par("station_top_start_x").doubleValue();
    _station_bot_start_x = par("station_bot_start_x").doubleValue();
    _station_top_start_y = par("station_top_start_y").doubleValue();
    _station_bot_start_y = par("station_bot_start_y").doubleValue();
    _station_lx = par("station_lx").doubleValue();
    _station_ly = par("station_ly").doubleValue();
    _inter_station_distance = par("inter_station_distance").doubleValue();
    _heartbeat_interval = par("heart_beat_interval").doubleValue();
    _station_init_time_gap = par("station_init_time_gap").doubleValue();

    // Draw the top and bottom road surfaces
    drawRoad(std::string("road_top"), _road_top_start_x, _road_top_start_y, _road_lx, _road_ly);
    drawRoad(std::string("road_bot"), _road_bot_start_x, _road_bot_start_y, _road_lx, _road_ly);

    _counter_station = 0;
    _setup_trigger_top = new omnetpp::cMessage("setup_sensor_station_top");
    _setup_trigger_bot = new omnetpp::cMessage("setup_sensor_station_bot");
    scheduleAt(omnetpp::simTime(), _setup_trigger_top);

    _heartbeat_trigger = new omnetpp::cMessage("heartbeat");
    scheduleAfter(_heartbeat_interval, _heartbeat_trigger);
}

void TestWorld::handleMessage(omnetpp::cMessage *msg)
{
    if(msg == _setup_trigger_top)
    {
        setupSensorstation(_counter_station, ConvoyDirection::TOP);
        setupStationBackendInterface(_counter_station, ConvoyDirection::TOP);
        scheduleAfter(_station_init_time_gap, _setup_trigger_bot);
    }
    if(msg == _setup_trigger_bot)
    {
        setupSensorstation(_counter_station, ConvoyDirection::DOWN);
        setupStationBackendInterface(_counter_station, ConvoyDirection::DOWN);
        if((++_counter_station) < _n_stations)
            scheduleAfter(_station_init_time_gap, _setup_trigger_top);
    }
    else if(msg == _heartbeat_trigger)
        scheduleAfter(_heartbeat_interval, _heartbeat_trigger);
}

void TestWorld::drawRoad(std::string name, double x, double y, double lx, double ly)
{
    omnetpp::cCanvas *world_canvas = getParentModule()->getCanvas();
    omnetpp::cRectangleFigure *road = new omnetpp::cRectangleFigure(name.c_str());
    road->setBounds(omnetpp::cFigure::Rectangle(x,y,lx,ly));
    road->setLineColor(omnetpp::cFigure::Color("white"));
    road->setLineWidth(2);
    road->setFillColor(omnetpp::cFigure::Color("grey50"));
    road->setFilled(true);
    road->setZIndex(-1);
    world_canvas->addFigure(road);
}

void TestWorld::setupSensorstation(int station_id, ConvoyDirection direction)
{
    // Work out location
    double x, y;
    switch(direction)
    {
    case ConvoyDirection::TOP:
        x = _station_top_start_x + (_inter_station_distance * station_id);
        y = _station_top_start_y;
        break;
    case ConvoyDirection::DOWN:
        x = _station_bot_start_x + (_inter_station_distance * station_id);
        y = _station_bot_start_y;
        break;
    default:
        break;
    }

    // Setup station
    createInitSensorStation(station_id, (x + _station_lx/2), (y + _station_ly/2), direction);
}

void TestWorld::createInitSensorStation(int station_id, double x, double y, ConvoyDirection direction)
{
    // Instantiate and setup sensor station
    std::string module_type_name = std::string("convoy_architecture.nodes.WrapperNode");
    std::string module_name = std::string("d") + std::to_string((int)direction) + std::string("rsu");
    omnetpp::cModuleType *module_type = omnetpp::cModuleType::get(module_type_name.c_str());

    // Create submodule vector first, if it doesn't already exist
    if(!getParentModule()->hasSubmoduleVector(module_name.c_str()))
        getParentModule()->addSubmoduleVector(module_name.c_str(), _n_stations);

    // Create the sensor station module
    omnetpp::cModule *module = module_type->create(module_name.c_str(), getParentModule(), station_id);

    // Set position
    omnetpp::cDisplayString display_string = module->getDisplayString();
    display_string.setTagArg("p", 0, x);
    display_string.setTagArg("p", 1, y);
    /*
    display_string.setTagArg("b", 0, _station_lx/3);
    display_string.setTagArg("b", 1, (_station_ly*2)/3);
    display_string.setTagArg("b", 4, "white");
    display_string.setTagArg("b", 5, 1);
    */
    module->setDisplayString(display_string.str());
    module->setDisplayName(nullptr);

    // Finalize module
    module->par("stationID") = (module_name + std::string("[") + std::to_string(station_id) + std::string("]")).c_str();
    switch(direction)
    {
    case ConvoyDirection::DOWN:
        module->par("detectorFovTag") = "down";
        module->par("detectorFovLimitAngleMin") = 0.00;
        module->par("detectorFovLimitAngleMax") = 3.13;
        break;
    default:
        module->par("detectorFovTag") = "top";
        module->par("detectorFovLimitAngleMin") = 3.14;
        module->par("detectorFovLimitAngleMax") = 6.27;
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
    _sensor_station.push_back(module);
}

void TestWorld::setupStationBackendInterface(int station_id, ConvoyDirection direction)
{
    omnetpp::cModule *bk_if_module;
    switch(direction)
    {
    case ConvoyDirection::TOP:
        bk_if_module = getParentModule()->getSubmodule("bk_if_top", station_id);
        break;
    case ConvoyDirection::DOWN:
        bk_if_module = getParentModule()->getSubmodule("bk_if_bot", station_id);
        break;
    default:
        break;
    }

    // Connect sensor station to backend interface
    _sensor_station.back()->gate("outAppDtwinPubBknd")->connectTo(bk_if_module->gate("inAppDtwin"));
    bk_if_module->gate("outAppConvoyOrch")->connectTo(_sensor_station.back()->gate("inAppConvoyOrchBknd"));
}
} // namespace convoy_architecture
