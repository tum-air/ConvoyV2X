/*
 * ConvoyArchitectureDefs.h
 *
 *  Created on: Apr 14, 2024
 *      Author: venkat
 */

#ifndef SRC_COMMON_CONVOYARCHITECTUREDEFS_H_
#define SRC_COMMON_CONVOYARCHITECTUREDEFS_H_

#include "veins_inet/VeinsInetMobility.h"

namespace lte {

enum ConvoyDirection {UNDEFINED, TOP, DOWN};

struct Publication {
    std::string name {std::string("")};
    int id {0};
    inet::Coord position {0.0, 0.0, 0.0};
    double fov {0.0};
    ConvoyDirection direction {ConvoyDirection::UNDEFINED};
    int cluster;
};

struct Broadcast {
    int id_convoy {0};
    int id_cluster {0};
    bool full_convoy {false};
    bool full_cluster {false};
    ConvoyDirection direction {ConvoyDirection::UNDEFINED};
    double rssi {0.0};
};

} // namespace lte

#endif /* SRC_COMMON_CONVOYARCHITECTUREDEFS_H_ */
