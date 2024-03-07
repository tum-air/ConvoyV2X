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
    std::string name;
    int id_gnb;
    int id_ue;
    int id_gnb_gw;
    int id_ue_gw;
    StationType type;
    Role role;
    inet::Coord position;
    double speed;
    double txp;
    double txp_gw;
    int direction;
    int id_convoy;
    int id_cluster;
    uint64_t timestamp;
};

struct Cluster {
    std::string name;
    int id;
    std::vector<Node> nodes;
    double length;
};

struct Convoy {
    std::string name;
    int id;
    std::vector<Cluster> clusters;
    double length;
};

} // namespace convoy_architecture

#endif /* COMMON_DEFS_H_ */
