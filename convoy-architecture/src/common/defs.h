/*
 * defs.h
 *
 *  Created on: Jun 8, 2023
 *      Author: simu5g
 */

#ifndef COMMON_DEFS_H_
#define COMMON_DEFS_H_

#include "veins_inet/VeinsInetMobility.h"

namespace convoy_architecture {

enum StationType {VEHICLE,RSU};
enum ConvoyDirection {UNDEFINED, TOP, DOWN};
enum Role {MANAGER, MEMBER, GATEWAY};

struct Node {
    std::string name{std::string("")};
    int id_gnb {0};
    int id_ue {0};
    int id_gnb_gw {0};
    int id_ue_gw {0};
    StationType type {StationType::RSU};
    Role role {Role::MEMBER};
    inet::Coord position;
    double speed {0};
    double txp {0};
    double txp_gw {0};
    int direction {0};
    int id_convoy {0};
    int id_cluster {0};
    uint64_t timestamp {0};
};

struct Cluster {
    std::string name {std::string("")};
    int id {0};
    std::vector<Node> nodes {};
    double length {0};
};

struct Convoy {
    std::string name {std::string("")};
    int id {0};
    std::vector<Cluster> clusters {};
    double length {0};
};

} // namespace convoy_architecture

#endif /* COMMON_DEFS_H_ */
