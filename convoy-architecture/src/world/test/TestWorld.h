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

#ifndef __CONVOY_ARCHITECTURE_TESTWORLD_H_
#define __CONVOY_ARCHITECTURE_TESTWORLD_H_

#include <omnetpp.h>
#include <string.h>
#include "common/defs.h"

namespace convoy_architecture {

/**
 * TODO - Generated class
 */
class TestWorld : public omnetpp::cSimpleModule
{
  private:
    int _n_stations;
    double _road_top_start_x;
    double _road_top_start_y;
    double _road_bot_start_x;
    double _road_bot_start_y;
    double _road_lx;
    double _road_ly;
    double _station_top_start_x;
    double _station_bot_start_x;
    double _station_top_start_y;
    double _station_bot_start_y;
    double _station_lx;
    double _station_ly;
    double _inter_station_distance;
    double _heartbeat_interval;
    double _station_init_time_gap;
    int _counter_station;
    std::vector<omnetpp::cModule*> _sensor_station;
    omnetpp::cMessage* _setup_trigger_top;
    omnetpp::cMessage* _setup_trigger_bot;
    omnetpp::cMessage* _setup_trigger_bknd;
    omnetpp::cMessage* _heartbeat_trigger;
    omnetpp::cModule* _bknd_switch_top;
    omnetpp::cModule* _bknd_interface_top;
    omnetpp::cChannel* _bknd_eth_chnl_1_top;
    omnetpp::cChannel* _bknd_eth_chnl_2_top;
    omnetpp::cModule* _bknd_module_top;
    omnetpp::cModule* _bknd_switch_bot;
    omnetpp::cModule* _bknd_interface_bot;
    omnetpp::cChannel* _bknd_eth_chnl_1_bot;
    omnetpp::cChannel* _bknd_eth_chnl_2_bot;
    omnetpp::cModule* _bknd_module_bot;
    std::vector<omnetpp::cModule*> _station_backend_interface;
    std::vector<omnetpp::cChannel*> _station_eth_chnl_1;
    std::vector<omnetpp::cChannel*> _station_eth_chnl_2;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    void drawRoad(std::string, double, double, double, double);
    void setupSensorstation(int, ConvoyDirection);
    void createInitSensorStation(int, double, double, ConvoyDirection);
    void setupBackendTop();
    void setupBackendDown();
    void setupStationBackendInterface(int, ConvoyDirection);
  public:
    TestWorld();
    ~TestWorld();
};

} // namespace convoy_architecture
#endif
