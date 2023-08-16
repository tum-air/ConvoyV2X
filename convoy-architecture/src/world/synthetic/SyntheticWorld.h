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

#ifndef __CONVOY_ARCHITECTURE_SYNTHETICWORLD_H_
#define __CONVOY_ARCHITECTURE_SYNTHETICWORLD_H_

#include <omnetpp.h>
#include <string.h>

namespace convoy_architecture {
/**
 * TODO - Generated class
 */
class SyntheticWorld : public omnetpp::cSimpleModule
{
private:
    int _n_stations;
    double _road_top_start_x;
    double _road_top_start_y;
    double _road_bot_start_x;
    double _road_bot_start_y;
    double _road_lx;
    double _road_ly;
    double _station_start_x;
    double _station_start_y;
    double _station_lx;
    double _station_ly;
    double _inter_station_distance;

protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    void draw_road(std::string, double, double, double, double);
    void draw_sensor_station(std::string, double, double, double, double);
    void initialize_sensor_station(int, double, double);
    void setup_tracker_app(omnetpp::cModule*);
    void setup_dtwin_store_app(omnetpp::cModule*);
};

} // namespace convoy_architecture
#endif
