
#ifndef __CONVOY_ARCHITECTURE_DTWINSTORE_H_
#define __CONVOY_ARCHITECTURE_DTWINSTORE_H_

#include <omnetpp.h>
#include "messages/ObjectList_m.h"

namespace convoy_architecture {

/**
 * Digital twin real-time store
 */
class DtwinStore : public omnetpp::cSimpleModule
{
protected:
    int _n_objects = 0;
    std::set<std::string> _object_id;
    std::map<std::string, std::string> _object_type;
    std::map<std::string, int> _object_address;
    std::map<std::string, double> _object_position_x;
    std::map<std::string, double> _object_position_y;
    std::map<std::string, double> _object_position_z;
    std::map<std::string, double> _object_heading;
    std::map<std::string, uint64_t> _object_timestamp;
    omnetpp::simtime_t _object_max_age;
    omnetpp::simtime_t _object_expiry_check_interval;
    omnetpp::cMessage *_object_expiry_check_event{nullptr};

  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    virtual void checkObjectExpiry();
    virtual void updateObjects(ObjectList *tracked_objects);

  public:
    ~DtwinStore();
    ObjectList* readFromStore();
    std::set<std::string> readIDs();
    std::string readType(std::string object_id);
    double readPositionX(std::string object_id);
    double readPositionY(std::string object_id);
    double readPositionZ(std::string object_id);
    double readHeading(std::string object_id);
    uint64_t readTimestamp(std::string object_id);
    void updateObjectTypeAndAddress(std::string object_id, std::string object_type, int object_address);
};
} // namespace convoy_architecture

#endif
