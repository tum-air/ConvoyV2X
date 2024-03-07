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

message DtwinSub {
    uint64_t timestamp;
    string subscriber_id;
    string subscriber_type;
    int subscriber_address;
    string roi_tag;
    string qos_tag;
    
    string ccs_report_name;
    int ccs_report_direction;
    int ccs_report_convoy_id;
    int ccs_report_cluster_id;
    int ccs_report_id_gnb;
    int ccs_report_id_ue;
    int ccs_report_id_gnb_gw;
    int ccs_report_id_ue_gw;
    int ccs_report_type;
    int ccs_report_role;
    double ccs_report_position_x;
    double ccs_report_position_y;
    double ccs_report_position_z=0;
    double ccs_report_speed;
    double ccs_report_txp;
    double ccs_report_txp_gw;
    int ccs_report_id_convoy;
    int ccs_report_id_cluster;
}