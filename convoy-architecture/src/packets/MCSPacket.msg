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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import messages.ObjectList;
import messages.DtwinSub;
import messages.CoopManeuver;
import messages.ConvoyControlService;

class MCSPacket extends inet::FieldsChunk
{
    uint64_t timestamp;  
    int src_cluster;
    int hop_cluster;
    int dst_cluster;
    int src_mac_id;
    int hop_mac_id;
    int dst_mac_id;
    
    convoy_architecture::DtwinSub msg_dtwin_sub;
    convoy_architecture::ObjectList msg_dtwin_pub;
    convoy_architecture::CoopManeuver msg_coop_man;
    convoy_architecture::ConvoyControlService msg_ccs;
}