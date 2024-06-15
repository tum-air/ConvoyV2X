
package convoy_architecture.apps;

//
// Subscriber application for digital twin information
//

simple Subscriber
{
    parameters:
        volatile string subscriberID  = default("rsu_001");
        volatile string roiTag = default("horizon");
        volatile string qosTag = default("ASIL-D");
        volatile double startTime @unit(s) = default(100.000s);
        volatile double stopTime @unit(s) = default(1000.000s);
        volatile double updateRate @unit(s) = default(1.000s);
        volatile string subscriberType = default("v2v");
        @application;
        @display("i=block/sink");
        @class("convoy_architecture::Subscriber");

    gates:
        input in;
        output out_sub;
        output out_dtwin;
}