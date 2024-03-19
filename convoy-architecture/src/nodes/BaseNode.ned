
package convoy_architecture.nodes;

//
// Generic base node
//

import inet.mobility.contract.IMobility;
import convoy_architecture.apps.Detector;
import convoy_architecture.apps.Tracker;
import convoy_architecture.stores.DtwinStore;
import convoy_architecture.apps.Subscriber;
import convoy_architecture.apps.Localizer;
import convoy_architecture.apps.Publisher;
import convoy_architecture.apps.Maneuver;
import convoy_architecture.controls.RoutingControl;
import convoy_architecture.stores.SubscriberStore;

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
        @display("i=device/server;bgb=500,450;is=s");
        @class(convoy_architecture::BaseNode);

    gates:
        output outLlPublisher;
        input inLlPublisher;
        output outLlSubscriber;
        input inLlSubscriber;
        output outLlManeuver;
        input inLlManeuver;
        
        output outLlMemberControl;
        input inLlMemberControl;
        output outLlMemberReport;
        input inLlMemberReport;
        
        output outBkndPublisher;
        output outBkndMemberReport;
        input inBkndMemberControl;

    submodules:
        mobility: <mobilityType> like IMobility {
                @display("p=100,50;is=s");
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
            @display("p=250,150;is=s");
        }
        dtwinStore: DtwinStore if hasTracker || hasDtwinSub {
                @display("p=100,150;is=s");
        }
        appSubscriber: Subscriber if hasDtwinSub {
            @display("p=250,250;is=s");
            subscriberType = parent.subscriberType;
        }
        appLocalizer: Localizer if hasTracker || hasDtwinSub {
            @display("p=400,50;is=s");
            stationType = parent.detectorStationType;
        }
        appPublisher: Publisher if hasDtwinPub {
            @display("p=400,250;is=s");
        }
        appManeuver: Maneuver if hasCoopMan {
            @display("p=100,250;is=s");
        }
        routingControl: RoutingControl {
            @display("p=250,350;is=s");
            stationType = parent.stationType;
            convoyDirection = parent.convoyDirection;
        }
        subscriberStore: SubscriberStore if hasDtwinPub {
                @display("p=400,150;is=s");
        }

    connections allowunconnected:
        appDetector.detectionsEgo --> appTracker.detectionsEgo if hasTracker;
        dtwinStore.in <-- appTracker.dtwinCollective if hasTracker;
        appTracker.dtwinNeighbours <-- appSubscriber.out_dtwin if hasTracker && hasDtwinSub;
        dtwinStore.in <-- appSubscriber.out_dtwin if (!hasTracker) && hasDtwinSub;
        subscriberStore.in <-- appPublisher.outSubscriberStore if hasDtwinPub;

        routingControl.outUlSubscriber --> appSubscriber.in if hasDtwinSub;
        routingControl.inUlSubscriber <-- appSubscriber.out_sub if hasDtwinSub;
        routingControl.outUlPublisher --> appPublisher.in if hasDtwinPub;
        routingControl.inUlPublisher <-- appPublisher.out if hasDtwinPub;
        routingControl.outUlManeuver --> appManeuver.in if hasCoopMan;
        routingControl.inUlManeuver <-- appManeuver.out if hasCoopMan;
        
        routingControl.outLlSubscriber --> outLlSubscriber;
        routingControl.inLlSubscriber <-- inLlSubscriber;
        routingControl.outLlPublisher --> outLlPublisher;
        routingControl.inLlPublisher <-- inLlPublisher;
        routingControl.outLlManeuver --> outLlManeuver;
        routingControl.inLlManeuver <-- inLlManeuver;
        
        routingControl.outLlMemberControl --> outLlMemberControl;
        routingControl.inLlMemberControl <-- inLlMemberControl;
        routingControl.outLlMemberReport --> outLlMemberReport;
        routingControl.inLlMemberReport <-- inLlMemberReport;
        
        routingControl.outBkndPublisher --> outBkndPublisher;
        routingControl.outBkndMemberReport --> outBkndMemberReport;
        routingControl.inBkndMemberControl <-- inBkndMemberControl;
}
