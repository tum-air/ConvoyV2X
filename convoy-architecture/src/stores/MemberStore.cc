
#include "MemberStore.h"

namespace convoy_architecture {

Define_Module(MemberStore);

void MemberStore::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - MemberStore::initialize(): " << "Initializing member store " << std::endl;

    _member_expiry_check_event = new omnetpp::cMessage("memberExpiryCheck");
    scheduleAfter(par("memberCheckInterval").doubleValue(), _member_expiry_check_event);

    EV_INFO << current_time <<" - MemberStore::initialize(): " << "Initialized member store for station " << this->getParentModule()->getFullName() << std::endl;
}

void MemberStore::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    // Check if the message is triggered internally
    if (msg->isSelfMessage())
    {
        // Check for object-expiry event
        if (msg == _member_expiry_check_event)
        {
            EV_INFO << current_time <<" - MemberStore::handleMessage(): " << "Member-expiry check triggered " << std::endl;
            checkMemberExpiry();
            scheduleAfter(par("memberCheckInterval").doubleValue(), _member_expiry_check_event);
        }
    }
    else if(msg->arrivedOn("in"))
    {
        EV_INFO << current_time <<" - MemberStore::handleMessage(): " << "External message received through input gate " << std::endl;
        MemberStatus *msg_member_status = check_and_cast<MemberStatus *>(msg);
        updateNodeCCSReports(msg_member_status);
        delete msg_member_status;
    }
}

void MemberStore::updateNodeCCSReports(MemberStatus *msg) {
    // Edit existing entry if the node already exists and if the time stamp is more recent, else create a new entry
    Node member_info {
        std::string{msg->getCcs_report_name()},
        msg->getCcs_report_id_gnb(),
        msg->getCcs_report_id_ue(),
        msg->getCcs_report_id_gnb_gw(),
        msg->getCcs_report_id_ue_gw(),
        (StationType) msg->getCcs_report_type(),
        (Role) msg->getCcs_report_role(),
        inet::Coord{msg->getCcs_report_position_x(), msg->getCcs_report_position_y(), msg->getCcs_report_position_z()},
        msg->getCcs_report_speed(),
        msg->getCcs_report_txp(),
        msg->getCcs_report_txp_gw(),
        msg->getCcs_report_direction(),
        msg->getCcs_report_id_convoy(),
        msg->getCcs_report_id_cluster(),
        msg->getTimestamp()
    };

    std::string node_name = member_info.name;
    auto it = std::find_if(std::begin(_member_ccs_record), std::end(_member_ccs_record), [&node_name] (Node const& val) {return val.name == node_name;});
    if(it != std::end(_member_ccs_record)) {
        if((*it).timestamp < member_info.timestamp)
            (*it) = member_info;
    }
    else
        _member_ccs_record.push_back(member_info);
}

void MemberStore::checkMemberExpiry() {
    omnetpp::simtime_t current_time = omnetpp::simTime();
    uint64_t current_nsec = (uint64_t) current_time.raw();
    omnetpp::simtime_t max_age_sec = par("memberMaxAge").doubleValue();
    uint64_t max_age_nsec = (uint64_t) max_age_sec.raw();

    _member_ccs_record.erase(std::remove_if(std::begin(_member_ccs_record), std::end(_member_ccs_record),
            [&current_nsec, &max_age_nsec] (Node const& val) {return (current_nsec - val.timestamp) >= max_age_nsec;}),
            std::end(_member_ccs_record));
}

const std::vector<Node>& MemberStore::readCCSReports() const
{
    return _member_ccs_record;
}

} // namespace convoy_architecture
