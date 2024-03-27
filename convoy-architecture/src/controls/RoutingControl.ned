
package convoy_architecture.controls;

//
// Routing control module
//

simple RoutingControl
{
    parameters:
        int stationType = default(0);
        int convoyDirection = default(0);
        double roiHorizon @unit(m) = default(1000.0m);
        double roiNearby @unit(m) = default(300.0m);
        int sizeSubscriberMsg @unit(B)= default(10B);
        int sizeManeuverMsg @unit(B)= default(20B);
        int sizeMemberReportMsg @unit(B)= default(10B);
        int sizeOrchestrationMsg @unit(B)= default(10B);
        int destPortPublisher = default(21001);
        int destPortSubscriber = default(21002);
        int destPortManeuver = default(21003);
        int destPortMemberReport = default(21004);
        int destPortOrchestration = default(21005);
        @display("i=block/control");
        @class("convoy_architecture::RoutingControl");
    
    gates:
        output outUlPublisher;
        input inUlPublisher;
        output outUlSubscriber;
        input inUlSubscriber;
        output outUlManeuver;
        input inUlManeuver;
        
        output outLlPublisher;
        input inLlPublisher;
        output outLlSubscriber;
        input inLlSubscriber;
        output outLlManeuver;
        input inLlManeuver;
        
        output outUlMemberControl;
        input inUlMemberControl;
        input inUlMemberReport;
        
        output outLlMemberControl;
        input inLlMemberControl;
        output outLlMemberReport;
        input inLlMemberReport;
        
        output outBkndPublisher;
        output outBkndMemberReport;
        input inBkndMemberControl;
        
        // gateway ports
        output outLlPublisherGw;
        input inLlPublisherGw;
        output outLlSubscriberGw;
        input inLlSubscriberGw;
        output outLlManeuverGw;
        input inLlManeuverGw;
        
        output outUlMemberControlGw;
        input inUlMemberControlGw;
        input inUlMemberReportGw;
}
