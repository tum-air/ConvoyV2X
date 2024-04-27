/*
 * defs.h
 *
 *  Created on: Jun 8, 2023
 *      Author: simu5g
 */

#ifndef COMMON_DEFS_H_
#define COMMON_DEFS_H_

#include "veins_inet/VeinsInetMobility.h"
#include "common/ConvoyArchitectureDefs.h"

namespace convoy_architecture {

enum StationType {VEHICLE, RSU, BACKEND};
typedef lte::ConvoyDirection ConvoyDirection;
enum Role {UNDEFINED, MANAGER, MEMBER, GATEWAY};
enum MessageType {UNKNOWN, SUBSCRIPTION, PUBLICATION, MANEUVER, MEMBER_STATUS, ORCHESTRATION};
enum MemberControlState {INITIALIZING, INITIALIZED};
enum ClusterDeviceType {UNKNOWN_DEVICE, MASTER_DEVICE, MEMBER_DEVICE, GATEWAY_DEVICE};
enum DeviceInterfaceType {UNKNOWN_INTERFACE, PRIMARY_INTERFACE, GATEWAY_INTERFACE};

struct Node {
    std::string name{std::string("")};
    int address {0};
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

struct Subscription {
    std::string name {std::string("")};
    int id {0};
    std::string roi {std::string("")};
    std::string qos {std::string("")};
    uint64_t timestamp {0};
};

typedef lte::Publication Publication;

typedef lte::Broadcast Broadcast;

} // namespace convoy_architecture

#endif /* COMMON_DEFS_H_ */
