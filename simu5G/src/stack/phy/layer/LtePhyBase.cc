//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "stack/phy/layer/LtePhyBase.h"
#include "common/LteCommon.h"

using namespace omnetpp;

short LtePhyBase::airFramePriority_ = 10;

// convoy architecture - register signals
simsignal_t LtePhyBase::clusterBroadcastReceivedSignal = registerSignal("clusterBroadcastReceivedSignal");

LtePhyBase::LtePhyBase()
{
    primaryChannelModel_ = nullptr;
}

LtePhyBase::~LtePhyBase()
{
}

void LtePhyBase::initialize(int stage)
{
    ChannelAccess::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL)
    {
        binder_ = getBinder();
        cellInfo_ = nullptr;
        // get gate ids
        upperGateIn_ = findGate("upperGateIn");
        upperGateOut_ = findGate("upperGateOut");
        radioInGate_ = findGate("radioIn");

        // Initialize and watch statistics
        numAirFrameReceived_ = numAirFrameNotReceived_ = 0;
        ueTxPower_ = par("ueTxPower");
        eNodeBtxPower_ = par("eNodeBTxPower");
        microTxPower_ = par("microTxPower");

        carrierFrequency_ = 2.1e+9;
        WATCH(numAirFrameReceived_);
        WATCH(numAirFrameNotReceived_);

        multicastD2DRange_ = par("multicastD2DRange");
        enableMulticastD2DRangeCheck_ = par("enableMulticastD2DRangeCheck");

        WATCH(_convoy_direction);
    }
    else if (stage == inet::INITSTAGE_PHYSICAL_LAYER)
    {
        initializeChannelModel();
    }
}

void LtePhyBase::handleMessage(cMessage* msg)
{
    EV << " LtePhyBase::handleMessage - new message received" << endl;

    if (msg->isSelfMessage())
    {
        handleSelfMessage(msg);
    }
    // AirFrame
    else if (msg->getArrivalGate()->getId() == radioInGate_)
    {
        handleAirFrame(msg);
    }

    // message from stack
    else if (msg->getArrivalGate()->getId() == upperGateIn_)
    {
        handleUpperMessage(msg);
    }
    // unknown message
    else
    {
        EV << "Unknown message received." << endl;
        delete msg;
    }
}

void LtePhyBase::handleControlMsg(LteAirFrame *frame,
    UserControlInfo *userInfo)
{
    auto pkt = check_and_cast<inet::Packet *>(frame->decapsulate());
    delete frame;
    *(pkt->addTagIfAbsent<UserControlInfo>()) = *userInfo;
    delete userInfo;
    send(pkt, upperGateOut_);
    return;
}

LteAirFrame *LtePhyBase::createHandoverMessage()
{
    // broadcast airframe
    LteAirFrame *bdcAirFrame = new LteAirFrame("handoverFrame");
    UserControlInfo *cInfo = new UserControlInfo();
    cInfo->setIsBroadcast(true);
    cInfo->setIsCorruptible(false);
    cInfo->setSourceId(nodeId_);
    cInfo->setFrameType(HANDOVERPKT);
    cInfo->setTxPower(txPower_);
    cInfo->setCarrierFrequency(primaryChannelModel_->getCarrierFrequency());
    cInfo->setIsNr(isNr_);
    bdcAirFrame->setControlInfo(cInfo);
    bdcAirFrame->setDuration(0);
    bdcAirFrame->setSchedulingPriority(airFramePriority_);
    // current position
    cInfo->setCoord(getRadioPosition());
    return bdcAirFrame;
}

void LtePhyBase::handleUpperMessage(cMessage* msg)
{
    EV << "LtePhy: message from stack" << endl;

    auto pkt = check_and_cast<inet::Packet *>(msg);
    auto lteInfo = pkt->removeTag<UserControlInfo>();

    LteAirFrame* frame = nullptr;

    if (lteInfo->getFrameType() == HARQPKT
        || lteInfo->getFrameType() == GRANTPKT
        || lteInfo->getFrameType() == RACPKT
        || lteInfo->getFrameType() == D2DMODESWITCHPKT)
    {
        frame = new LteAirFrame("harqFeedback-grant");
    }
    else
    {
        // create LteAirFrame and encapsulate the received packet
        frame = new LteAirFrame("airframe");
    }

    frame->encapsulate(check_and_cast<cPacket*>(msg));

    // initialize frame fields
    if (lteInfo->getFrameType() == D2DMODESWITCHPKT)
        frame->setSchedulingPriority(-1);
    else
        frame->setSchedulingPriority(airFramePriority_);

    // set transmission duration according to the numerology
    NumerologyIndex numerologyIndex = binder_->getNumerologyIndexFromCarrierFreq(lteInfo->getCarrierFrequency());
    double slotDuration = binder_->getSlotDurationFromNumerologyIndex(numerologyIndex);
    frame->setDuration(slotDuration);

    // set current position
    lteInfo->setCoord(getRadioPosition());
    lteInfo->setTxPower(txPower_);
    frame->setControlInfo(lteInfo.get()->dup());

    EV << "LtePhy: " << nodeTypeToA(nodeType_) << " with id " << nodeId_
       << " sending message to the air channel. Dest=" << lteInfo->getDestId() << endl;
    sendUnicast(frame);
}

void LtePhyBase::initializeChannelModel()
{
    std::string moduleName = (strcmp(getFullName(), "nrPhy") == 0) ? "nrChannelModel" : "channelModel";
    primaryChannelModel_ = check_and_cast<LteChannelModel*>(getParentModule()->getSubmodule(moduleName.c_str(),0));
    primaryChannelModel_->setPhy(this);
    double carrierFreq = primaryChannelModel_->getCarrierFrequency();
    unsigned int numerologyIndex = primaryChannelModel_->getNumerologyIndex();
    channelModel_[carrierFreq] = primaryChannelModel_;

    if (nodeType_ == UE)
        binder_->registerCarrierUe(carrierFreq, numerologyIndex, nodeId_);

    int vectSize = primaryChannelModel_->getVectorSize();
    LteChannelModel* chanModel = NULL;
    for (int index=1; index<vectSize; index++)
    {
        chanModel = check_and_cast<LteChannelModel*>(getParentModule()->getSubmodule(moduleName.c_str(),index));
        chanModel->setPhy(this);
        carrierFreq = chanModel->getCarrierFrequency();
        numerologyIndex = chanModel->getNumerologyIndex();
        channelModel_[carrierFreq] = chanModel;
        if (nodeType_ == UE)
            binder_->registerCarrierUe(carrierFreq, numerologyIndex, nodeId_);
    }
}

void LtePhyBase::updateDisplayString()
{
    char buf[80] = "";
    if (numAirFrameReceived_ > 0)
        sprintf(buf + strlen(buf), "af_ok:%d ", numAirFrameReceived_);
    if (numAirFrameNotReceived_ > 0)
        sprintf(buf + strlen(buf), "af_no:%d ", numAirFrameNotReceived_);
    getDisplayString().setTagArg("t", 0, buf);
}

void LtePhyBase::sendBroadcast(LteAirFrame *airFrame)
{
    // delegate the ChannelControl to send the airframe
    sendToChannel(airFrame);
}

LteAmc *LtePhyBase::getAmcModule(MacNodeId id)
{
    LteAmc *amc = nullptr;
    OmnetId omid = binder_->getOmnetId(id);
    if (omid == 0)
        return nullptr;

    amc = check_and_cast<LteMacEnb *>(
        getSimulation()->getModule(omid)->getSubmodule("cellularNic")->getSubmodule(
            "mac"))->getAmc();
    return amc;
}

void LtePhyBase::sendMulticast(LteAirFrame *frame)
{
    UserControlInfo *ci = check_and_cast<UserControlInfo *>(frame->getControlInfo());

    // get the group Id
    int32_t groupId = ci->getMulticastGroupId();
    if (groupId < 0)
        throw cRuntimeError("LtePhyBase::sendMulticast - Error. Group ID %d is not valid.", groupId);

    // send the frame to nodes belonging to the multicast group only
    std::map<int, OmnetId>::const_iterator nodeIt = binder_->getNodeIdListBegin();
    for (; nodeIt != binder_->getNodeIdListEnd(); ++nodeIt)
    {
        MacNodeId destId = nodeIt->first;

        // if the node in the list does not use the same LTE/NR technology of this PHY module, skip it
        if (isNrUe(destId) != isNr_)
            continue;

        if (destId != nodeId_ && binder_->isInMulticastGroup(nodeIt->first, groupId))
        {
            EV << NOW << " LtePhyBase::sendMulticast - node " << destId << " is in the multicast group"<< endl;

            // get a pointer to receiving module
            cModule *receiver = getSimulation()->getModule(nodeIt->second);
            LtePhyBase * recvPhy;
            double dist;

            if( enableMulticastD2DRangeCheck_ )
            {
                // get the correct PHY layer module
                recvPhy =  (isNrUe(destId)) ? check_and_cast<LtePhyBase *>(receiver->getSubmodule("cellularNic")->getSubmodule("nrPhy"))
                                  : check_and_cast<LtePhyBase *>(receiver->getSubmodule("cellularNic")->getSubmodule("phy"));

                dist = recvPhy->getRadioPosition().distance(getRadioPosition());

                if( dist > multicastD2DRange_ )
                {
                    EV << NOW << " LtePhyBase::sendMulticast - node too far (" << dist << " > " << multicastD2DRange_ << ". skipping transmission" << endl;
                    continue;
                }
            }

            EV << NOW << " LtePhyBase::sendMulticast - sending frame to node " << destId << endl;

            sendDirect(frame->dup(), 0, frame->getDuration(), receiver, getReceiverGateIndex(receiver, isNrUe(destId)));
        }
    }

    // delete the original frame
    delete frame;
}

void LtePhyBase::sendUnicast(LteAirFrame *frame)
{
    UserControlInfo *ci = check_and_cast<UserControlInfo *>(
        frame->getControlInfo());
    // dest MacNodeId from control info
    MacNodeId dest = ci->getDestId();
    // destination node (UE or ENODEB) omnet id
    try {
        binder_->getOmnetId(dest);
    } catch (std::out_of_range& e) {
        delete frame;
        return;         // make sure that nodes that left the simulation do not send
    }
    OmnetId destOmnetId = binder_->getOmnetId(dest);
    if (destOmnetId == 0){
        // destination node has left the simulation
        delete frame;
        return;
    }
    // get a pointer to receiving module
    cModule *receiver = getSimulation()->getModule(destOmnetId);

    sendDirect(frame, 0, frame->getDuration(), receiver, getReceiverGateIndex(receiver, isNrUe(dest)));
}

int LtePhyBase::getReceiverGateIndex(const omnetpp::cModule *receiver, bool isNr) const
{
    int gate = (isNr) ? receiver->findGate("nrRadioIn") : receiver->findGate("radioIn");
    if (gate < 0) {
        gate = receiver->findGate("lteRadioIn");
        if (gate < 0) {
            throw cRuntimeError("receiver \"%s\" has no suitable radio input gate",
                                receiver->getFullPath().c_str());
        }
    }
    return gate;
}

/* convoy architecture */
void LtePhyBase::setClusterBroadcastParameters( MacNodeId convoy_id,
                                                MacNodeId cluster_id,
                                                int convoy_direction,
                                                bool convoy_full,
                                                bool cluster_full)
{
    _convoy_id = convoy_id;
    _cluster_id = cluster_id;
    _convoy_direction = convoy_direction;
    _convoy_full = convoy_full;
    _cluster_full = cluster_full;
}

void LtePhyBase::setClusterBroadcastParameters( const std::vector<MacNodeId> &publisher_id,
                                                const std::vector<double> &publisher_x,
                                                const std::vector<double> &publisher_y,
                                                const std::vector<double> &publisher_fov,
                                                const std::vector<int> &publisher_direction)
{
    _publisher_id.resize(publisher_id.size());
    _publisher_x.resize(publisher_x.size());
    _publisher_y.resize(publisher_y.size());
    _publisher_fov.resize(publisher_fov.size());
    _publisher_direction.resize(publisher_direction.size());
    std::copy(publisher_id.begin(), publisher_id.end(), _publisher_id.begin());
    std::copy(publisher_x.begin(), publisher_x.end(), _publisher_x.begin());
    std::copy(publisher_y.begin(), publisher_y.end(), _publisher_y.begin());
    std::copy(publisher_fov.begin(), publisher_fov.end(), _publisher_fov.begin());
    std::copy(publisher_direction.begin(), publisher_direction.end(), _publisher_direction.begin());
}

LteAirFrame *LtePhyBase::createConvoyControlMessage()
{
    // broadcast airframe
    LteAirFrame *convCtrlAirFrame = new LteAirFrame("convoyControlFrame");
    UserControlInfo *cInfo = new UserControlInfo();
    cInfo->setIsBroadcast(true);
    cInfo->setIsCorruptible(false);
    cInfo->setSourceId(nodeId_);
    cInfo->setFrameType(BROADCASTPKT);
    cInfo->setTxPower(txPower_);
    cInfo->setCarrierFrequency(primaryChannelModel_->getCarrierFrequency());
    cInfo->setIsNr(isNr_);

    // set convoy control info
    cInfo->setConvoyID((int) _convoy_id);
    cInfo->setClusterID((int) nodeId_);
    cInfo->setConvoyFull(_convoy_full);
    cInfo->setClusterFull(_cluster_full);
    cInfo->setConvoyDirection(_convoy_direction);
    for(int i=0; i<_publisher_id.size(); i++)
    {
        cInfo->appendPublisherNodeID(_publisher_id.at(i));
        cInfo->appendPublisherNodeX(_publisher_x.at(i));
        cInfo->appendPublisherNodeY(_publisher_y.at(i));
        cInfo->appendPublisherFOV(_publisher_fov.at(i));
        cInfo->appendPublisherDirection(_publisher_direction.at(i));
    }

    convCtrlAirFrame->setControlInfo(cInfo);
    convCtrlAirFrame->setDuration(0);
    convCtrlAirFrame->setSchedulingPriority(airFramePriority_);
    // current position
    cInfo->setCoord(getRadioPosition());

    return convCtrlAirFrame;
}

void LtePhyBase::setTxPower(double tx_power)
{
    txPower_ = tx_power;
}
