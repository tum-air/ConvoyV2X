
#include "ConvoyOrchestration.h"
#include "stores/DtwinStore.h"
#include "stores/MemberStore.h"
#include "messages/ObjectList_m.h"
#include "messages/ConvoyControlService_m.h"
#include "veins_inet/VeinsInetMobility.h"

namespace convoy_architecture {

Define_Module(ConvoyOrchestration);

ConvoyOrchestration::~ConvoyOrchestration()
{
    cancelAndDelete(_start_event);
    _start_event = nullptr;
    cancelAndDelete(_update_event);
    _update_event = nullptr;
}

void ConvoyOrchestration::initialize()
{
    omnetpp::simtime_t current_time = omnetpp::simTime();
    EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Initializing convoy orchestration application " << std::endl;

    double start_time = par("startTime").doubleValue();

    _start_event = new omnetpp::cMessage("startEvent");
    _update_event = new omnetpp::cMessage("updateEvent");

    if(par("stopTime").doubleValue() > start_time)
    {
        omnetpp::simtime_t trigger_time = (current_time < start_time)? start_time : current_time;
        scheduleAt(trigger_time, _start_event);
        EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Scheduled convoy orchestration start for time " << trigger_time << "s" << std::endl;
    }

        EV_INFO << current_time <<" - ConvoyOrchestration::initialize(): " << "Initialized convoy orchestration application" << std::endl;
}

void ConvoyOrchestration::handleMessage(omnetpp::cMessage *msg)
{
    omnetpp::simtime_t current_time = omnetpp::simTime();

    if (msg->isSelfMessage())
    {
        if (msg == _start_event)
        {
            EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Starting convoy orchestration application" << std::endl;
        }
        else if (msg == _update_event)
        {
            EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Executing orchestration step" << std::endl;

            _current_state.clear();
            _desired_state.clear();
            estimate_actual_state();
            compute_desired_state();
            enforce_desired_state();
        }
        scheduleAfter(par("updateRate").doubleValue(), _update_event);
    }

    if ((current_time >= par("stopTime").doubleValue()) && _update_event->isScheduled())
    {
        EV_INFO << current_time <<" - ConvoyOrchestration::handleMessage(): " << "Stopping convoy orchestration application" << std::endl;
        cancelEvent(_update_event);
    }
}

void ConvoyOrchestration::estimate_actual_state() {
    /* TODO
    ObjectList* dtwins = _dtwin_store->readFromStore();
    if(dtwins != nullptr)
        delete dtwins;
     */
    MemberStore *member_store = check_and_cast<MemberStore*>(this->getParentModule()->getSubmodule("memberStore"));
    const std::vector<Node>& ccs_reports = member_store->readCCSReports();
    std::copy(std::begin(ccs_reports), std::end(ccs_reports), std::back_inserter(_current_state));
}

void ConvoyOrchestration::compute_desired_state() {
    /* TODO
     * Convoy orchestration strategy
     */
    std::copy(std::begin(_current_state), std::end(_current_state), std::back_inserter(_desired_state));
}

void ConvoyOrchestration::enforce_desired_state() {
    std::vector<ConvoyControlService*> ccs_msg;
    std::for_each(std::begin(_desired_state), std::end(_desired_state), [this] (const Node& n) {
        ConvoyControlService *ccs_msg = new ConvoyControlService("ConvoyOrchestration");
        ccs_msg->setNode_id(n.name.c_str());
        ccs_msg->setNode_address(n.address);
        /*
         * TODO
         * Set next hop address - message is always routed through the rsu
         * ccs_msg->setHop_address(? - obtain record of next hop from member store ?);
         */
        ccs_msg->setTimestamp((uint64_t) omnetpp::simTime().raw());
        send(ccs_msg, "out");
    });
}

} // namespace convoy_architecture
