
package convoy_architecture.apps;

//
// Publisher application for digital twin
//

simple Publisher
{
    parameters:
        string publisherID = default("rsu_001");
        double startTime @unit(s) = default(100.000s);
        double stopTime @unit(s) = default(1000.000s);
        double updateRate @unit(s) = default(0.100s);
        @application;
        @display("i=block/source");
        @class("convoy_architecture::Publisher");
        
    gates:
        input in;
        output out;
        output outSubscriberStore;
}