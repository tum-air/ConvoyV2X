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

//
// TODO generated message class
//
namespace convoy_architecture;

message ObjectList {
    uint64_t timestamp;
    string station_id;
    string station_type;
    double station_position_x;
    double station_position_y;
    double station_position_z = 0;
    double station_heading;
    int obj_byte_size = 20;
    int n_objects = 0;
    string object_id[];
    int object_address[];
    string object_type[];
    uint64_t object_timestamp[];
    double object_position_x[];
    double object_position_y[];
    double object_position_z[] = 0;
    double object_heading[];
}