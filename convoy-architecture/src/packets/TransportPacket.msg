
//
// Transport packet 
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import messages.DtwinSub;
import messages.ObjectList;
import messages.CoopManeuver;
import messages.MemberStatus;
import messages.Orchestration;

class TransportPacket extends inet::FieldsChunk
{
    uint64_t timestamp;  
    int src_cluster;
    int hop_cluster;
    int dst_cluster;
    int src_mac_id;
    int hop_mac_id;
    int dst_mac_id;
    
    convoy_architecture::DtwinSub msg_subscriber;
    convoy_architecture::ObjectList msg_publisher;
    convoy_architecture::CoopManeuver msg_maneuver;
    convoy_architecture::MemberStatus msg_member_status;
    convoy_architecture::Orchestration msg_orchestration;
}