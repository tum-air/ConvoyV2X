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

#include "SyntheticWorld.h"
#include <iomanip>
#include "inet/mobility/static/StationaryMobility.h"

namespace convoy_architecture {

Define_Module(SyntheticWorld);

void SyntheticWorld::initialize()
{
    _n_stations = par("n_stations").intValue();
    _road_top_start_x = par("road_top_start_x").doubleValue();
    _road_top_start_y = par("road_top_start_y").doubleValue();
    _road_bot_start_x = par("road_bot_start_x").doubleValue();
    _road_bot_start_y = par("road_bot_start_y").doubleValue();
    _road_lx = par("road_lx").doubleValue();
    _road_ly = par("road_ly").doubleValue();
    _station_start_x = par("station_start_x").doubleValue();
    _station_start_y = par("station_start_y").doubleValue();
    _station_lx = par("station_lx").doubleValue();
    _station_ly = par("station_ly").doubleValue();
    _inter_station_distance  = par("inter_station_distance").doubleValue();

    // Draw the top and bottom road surfaces
    this->draw_road(std::string("road_top"), _road_top_start_x, _road_top_start_y, _road_lx, _road_ly);
    this->draw_road(std::string("road_bot"), _road_bot_start_x, _road_bot_start_y, _road_lx, _road_ly);

    // Draw the sensor-stations
    for(int i=0; i<_n_stations; i++)
    {
        double x = _station_start_x + (_inter_station_distance * i);
        double y = _station_start_y;

        /* sensor station figure if required
        this->draw_sensor_station(std::string("sensor_station_") + std::to_string(i+1), x, y, _station_lx, _station_ly);
        */

        this->initialize_sensor_station(i, (x + _station_lx/2), (y + _station_ly/2));
    }
}

void SyntheticWorld::draw_road(std::string name, double x, double y, double lx, double ly)
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

void SyntheticWorld::draw_sensor_station(std::string name, double x, double y, double lx, double ly)
{
    omnetpp::cCanvas *world_canvas = getParentModule()->getCanvas();
    omnetpp::cRectangleFigure *station = new omnetpp::cRectangleFigure(name.c_str());
    station->setBounds(omnetpp::cFigure::Rectangle(x,y,lx,ly));
    station->setLineColor(omnetpp::cFigure::Color("white"));
    station->setLineWidth(1);
    station->setFillColor(omnetpp::cFigure::Color("red"));
    station->setFilled(true);
    station->setZIndex(-1);
    world_canvas->addFigure(station);

    omnetpp::cTextFigure *label = new omnetpp::cTextFigure(std::string("label_"+name).c_str());
    label->setText(name.c_str());
    label->setPosition(omnetpp::cFigure::Point(x-50,y-30));
    world_canvas->addFigure(label);
}

void SyntheticWorld::initialize_sensor_station(int station_id, double x, double y)
{
    // Instantiate and setup sensor station
    std::string module_type_name = std::string("convoy_architecture.nodes.Sensorstation");
    std::string module_name = std::string("ss");
    omnetpp::cModuleType *module_type = omnetpp::cModuleType::get(module_type_name.c_str());

    // Create submodule vector first, if it doesn't already exist
    if(!getParentModule()->hasSubmoduleVector(module_name.c_str()))
        getParentModule()->addSubmoduleVector(module_name.c_str(), _n_stations);

    // Create the sensor station module
    omnetpp::cModule *module = module_type->create(module_name.c_str(), getParentModule(), station_id);

    // Initialize submodules
    module->par("hasRNISupport") = false;
    module->par("numEthInterfaces") = 0;
    module->par("numLoInterfaces") = 1;
    module->par("numPcapRecorders") = 0;
    module->par("hasIpv6") = false;
    module->par("hasIpv4") = true;
    module->par("nicType") = "NRNicUe";
    module->par("masterId") = 0;
    module->par("hasSctp") = false;
    module->par("hasTcp") = false;
    module->par("hasUdp") = false;

    // Initialize tracker application submodule
    // this->setup_tracker_app(module);

    // Initialize dtwin_store application submodule
    // this->setup_dtwin_store_app(module);

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
    module->finalizeParameters();
    module->buildInside();
    module->scheduleStart(omnetpp::simTime());
}

void SyntheticWorld::setup_tracker_app(omnetpp::cModule* sensor_station)
{
    std::string module_type_name_tracker_app = "convoy_architecture.apps.Tracker";
    omnetpp::cModuleType *module_type_tracker_app = omnetpp::cModuleType::get(module_type_name_tracker_app.c_str());
    omnetpp::cModule* tracker_app = sensor_station->addSubmodule(module_type_tracker_app, "app", 0);

    // Set tracker parameters.
    // Unset parameters take the default value defined in the NED file
    // Gate connections are automatically made for the tracker app
    tracker_app->par("stationID") = sensor_station->getFullName();
    tracker_app->par("stationType") = "rsu";
}

void SyntheticWorld::setup_dtwin_store_app(omnetpp::cModule* sensor_station)
{
    std::string module_type_name_dtwin_store = "convoy_architecture.apps.DtwinStore";
    omnetpp::cModuleType *module_type_dtwin_store = omnetpp::cModuleType::get(module_type_name_dtwin_store.c_str());
    omnetpp::cModule* dtwin_store = sensor_station->addSubmodule(module_type_dtwin_store, "dtwin_store");

    // Set dtwin_store parameters.
    // Unset parameters take the default value defined in the NED file
    // Connect input gate to message dispatcher
    omnetpp::cGate* input_gate = dtwin_store->gate("socketIn");
    int output_gate_index = sensor_station->getSubmodule("at")->gateSize("out");
    omnetpp::cGate* output_gate = sensor_station->getSubmodule("at")->gate("out", output_gate_index);
    input_gate->connectTo(output_gate);
}

void SyntheticWorld::handleMessage(omnetpp::cMessage *msg)
{
    // TODO - Generated method body
}
} // namespace convoy_architecture
