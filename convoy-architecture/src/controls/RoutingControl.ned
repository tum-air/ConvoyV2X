
package convoy_architecture.controls;

//
// Routing control module
//

simple RoutingControl
{
    parameters:
        int stationType = default(0);
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
}
