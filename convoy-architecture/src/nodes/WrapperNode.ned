//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package convoy_architecture.nodes;

import inet.mobility.contract.IMobility;
import convoy_architecture.controls.MessagingControl;
import convoy_architecture.stores.DtwinStore;
import convoy_architecture.apps.Detector;
import convoy_architecture.apps.Tracker;
import convoy_architecture.apps.DtwinSubscriber;
import convoy_architecture.apps.DtwinPublisher;
import convoy_architecture.apps.CooperativeManeuver;
import convoy_architecture.apps.Localizer;
import convoy_architecture.interfaces.BackendMessaging;

//
// TODO auto-generated module
//
module WrapperNode
{
    parameters:
        string stationID @mutable = default("d1rsu[0]");
        int convoyDirection @mutable = default(0);
        int stationType = default(0);
        string mobilityType = default("VeinsInetMobility");
        bool hasDtwinSub = default(true);
        bool hasDtwinPub = default(true);
        bool hasCoopMan = default(false);
        bool hasTracker = default(true);
        string detectorStationType = default("vehicle");
        string detectorFovTag = default("down");
        double detectorFovLimitRange @unit(m) = default(100m);
        double detectorFovLimitAngleMin @unit(rad) = default(-3.14rad);
        double detectorFovLimitAngleMax @unit(rad) = default(3.14rad);
        string backend_interface_name = (convoyDirection < 2)? "backend_top" : "backend_bot";
        string subscriberType @mutable = default("v2i");
        @display("i=device/server;bgb=500,300;is=s");
        @class(convoy_architecture::WrapperNode);
    gates:
    	output outAppDtwinPub;
        input inAppDtwinPub;
        output outAppDtwinSub;
        input inAppDtwinSub;
        output outAppCoopMan;
        input inAppCoopMan;
        output outAppDtwinPubBknd;
        input inAppConvoyOrchBknd;    
    submodules:
        mobility: <mobilityType> like IMobility {
            @display("p=400,250;is=s");
        }
        mcsAgent: MessagingControl {
            @display("p=250,250;is=s");
            convoyDirection = parent.convoyDirection;
        }
        dtwinStore: DtwinStore if hasTracker || hasDtwinSub {
            @display("p=100,50;is=s");
        }
        appEgoDetector: Detector if hasTracker {
            @display("p=400,50;is=s");
            stationID = parent.stationID;
            stationType = parent.detectorStationType;
            fovTag = parent.detectorFovTag;
            fovLimitRange = parent.detectorFovLimitRange;
            fovLimitAngleMin = parent.detectorFovLimitAngleMin;
            fovLimitAngleMax = parent.detectorFovLimitAngleMax;
        }
        appTracker: Tracker if hasTracker {
            @display("p=250,50;is=s");
        }
        appDtwinSub: DtwinSubscriber if hasDtwinSub {
            @display("p=250,150;is=s");
            subscriberType = parent.subscriberType;
        }
        appDtwinPub: DtwinPublisher if hasDtwinPub {
            @display("p=100,150;is=s");
        }
        appCoopMan: CooperativeManeuver if hasCoopMan {
            @display("p=400,150;is=s");
        }
        appLocalizer: Localizer if hasTracker || hasDtwinSub {
            @display("p=400,200;is=s");
            stationType = parent.detectorStationType;
        }
        bmsAgent: BackendMessaging if hasDtwinPub {
            @display("p=100,250;is=s");
            destinationModuleDtwin = parent.backend_interface_name;
        }
    connections allowunconnected:
        mcsAgent.outLlAppDtwinPub --> outAppDtwinPub;
        mcsAgent.outLlAppDtwinSub --> outAppDtwinSub;
        mcsAgent.outLlAppCoopMan --> outAppCoopMan;
        inAppDtwinPub --> mcsAgent.inLlAppDtwinPub;
        inAppDtwinSub --> mcsAgent.inLlAppDtwinSub;
        inAppCoopMan --> mcsAgent.inLlAppCoopMan;
        
        mcsAgent.outUlAppDtwinSub --> appDtwinSub.in if hasDtwinSub;
        mcsAgent.outUlAppCoopMan --> appCoopMan.in if hasCoopMan;
        mcsAgent.inUlAppDtwinSub <-- appDtwinSub.out_sub if hasDtwinSub;
        mcsAgent.inUlAppDtwinPub <-- appDtwinPub.out if hasDtwinPub;
        mcsAgent.inUlAppCoopMan <-- appCoopMan.out if hasCoopMan;
        
        appEgoDetector.detectionsEgo --> appTracker.detectionsEgo if hasTracker;
		appTracker.dtwinNeighbours <-- appDtwinSub.out_dtwin if hasTracker && hasDtwinSub;
		
		dtwinStore.in <-- appTracker.dtwinCollective if hasTracker;
		dtwinStore.in <-- appDtwinSub.out_dtwin if (!hasTracker) && hasDtwinSub;
		
		appDtwinPub.out_bknd --> bmsAgent.inUlDtwin if hasDtwinPub;
		bmsAgent.outLlDtwin --> outAppDtwinPubBknd if hasDtwinPub;
		bmsAgent.outMcsAgent --> mcsAgent.inBmsAgent if hasDtwinPub;
		inAppConvoyOrchBknd --> bmsAgent.inLlConvoyOrch if hasDtwinPub;
}
