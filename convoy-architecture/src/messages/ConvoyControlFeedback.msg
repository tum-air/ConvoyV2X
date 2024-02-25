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

namespace convoy_architecture;

message CCSNodeFeedback {
    uint64_t timestamp;
    string node_name;
    int direction;
    int convoy_id;
    int cluster_id;
    int node_id_gnb;
    int node_id_ue;
    int node_id_gnb_gw;
    int node_id_ue_gw;
    int node_type;
    int node_role;
    double node_position_x;
    double node_position_y;
    double node_position_z=0;
    double node_speed;
    double node_txp;
    double node_txp_gw;
}

message CCSClusterFeedback {
    uint64_t timestamp;
    int direction;
    int convoy_id;
    int cluster_id;
}