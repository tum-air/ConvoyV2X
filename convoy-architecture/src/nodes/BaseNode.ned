
package convoy_architecture.nodes;

//
// Generic base node
//

import inet.mobility.contract.IMobility;
import convoy_architecture.apps.Detector;
import convoy_architecture.apps.Tracker;
import convoy_architecture.stores.DtwinStore;
import convoy_architecture.apps.Subscriber;

module BaseNode
{
    parameters:
        
        // Node properties
        bool hasDtwinSub = default(true);
        bool hasDtwinPub = default(true);
        bool hasCoopMan = default(false);
        bool hasTracker = default(true);
        string stationID = default("d1rsu[0]");
        int convoyDirection = default(0);
        int stationType = default(0);
        string mobilityType = default("VeinsInetMobility");
        string detectorStationType = default("vehicle");
        string detectorFovTag = default("down");
        double detectorFovLimitRange @unit(m) = default(100m);
        double detectorFovLimitAngleMin @unit(rad) = default(-3.14rad);
        double detectorFovLimitAngleMax @unit(rad) = default(3.14rad);
        string backend_interface_name = (convoyDirection < 2)? "backend_top" : "backend_bot";
        string subscriberType = default("v2i");
        
        // Module parameters
        @display("i=device/server;bgb=500,500;is=s");
        @class(convoy_architecture::BaseNode);

    submodules:
        mobility: <mobilityType> like IMobility {
                @display("p=50,50;is=s");
        }
        appDetector: Detector if hasTracker {
            @display("p=250,50;is=s");
            stationID = parent.stationID;
            stationType = parent.detectorStationType;
            fovTag = parent.detectorFovTag;
            fovLimitRange = parent.detectorFovLimitRange;
            fovLimitAngleMin = parent.detectorFovLimitAngleMin;
            fovLimitAngleMax = parent.detectorFovLimitAngleMax;
        }
        appTracker: Tracker if hasTracker {
            @display("p=400,50;is=s");
        }
        dtwinStore: DtwinStore if hasTracker || hasDtwinSub {
                @display("p=400,150;is=s");
        }
        appSubscriber: Subscriber if hasDtwinSub {
            @display("p=250,150;is=s");
            subscriberType = parent.subscriberType;
        }

    connections allowunconnected:
        appDetector.detectionsEgo --> appTracker.detectionsEgo if hasTracker;
        dtwinStore.in <-- appTracker.dtwinCollective if hasTracker;
        appTracker.dtwinNeighbours <-- appSubscriber.out_dtwin if hasTracker && hasDtwinSub;
        dtwinStore.in <-- appSubscriber.out_dtwin if (!hasTracker) && hasDtwinSub;
}
