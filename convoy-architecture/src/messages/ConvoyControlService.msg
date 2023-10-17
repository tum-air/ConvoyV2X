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

message ConvoyControlService {
    uint64_t timestamp;
    string node_id;
    int convoy_role;
    int cluster_role;
    int convoy_id;
    int cluster_id;
    int convoy_id_gw;
    int cluster_id_gw;
    double tx_power;
    double tx_power_gw;
    int max_rb;
}