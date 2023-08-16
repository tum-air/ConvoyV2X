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

package convoy_architecture.world.synthetic;

//
// Synthetic base world for simulation
//
simple SyntheticWorld
{
    parameters:
        @display("i=block/network2;is=s");
        @class("convoy_architecture::SyntheticWorld");
        int n_stations = default(15);
        double road_top_start_x @unit(m) = default(10m);
        double road_top_start_y @unit(m) = default(200m);
        double road_bot_start_x @unit(m) = default(10m);
        double road_bot_start_y @unit(m) = default(220m);
        double road_lx @unit(m) = default(4980m);
        double road_ly @unit(m) = default(13m);
        double station_start_x @unit(m) = default(400m);
        double station_start_y @unit(m) = default(213m);
        double station_lx @unit(m) = default(10m);
        double station_ly @unit(m) = default(7m);
        double inter_station_distance @unit(m) = default(300m);
}