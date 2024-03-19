//
// Membership feedback message 
//

namespace convoy_architecture;

message MemberStatus {
        uint64_t timestamp;
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