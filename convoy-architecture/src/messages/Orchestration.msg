
//
// Convoy orchestration message
//
namespace convoy_architecture;

message Orchestration {
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