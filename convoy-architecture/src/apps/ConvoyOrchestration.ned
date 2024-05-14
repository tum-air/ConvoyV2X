
package convoy_architecture.apps;

simple ConvoyOrchestration
{
    parameters:
        @application;
        @display("i=block/network2;is=s");
        @class("convoy_architecture::ConvoyOrchestration");
        double startTime @unit(s) = default(500.000s);
        double stopTime @unit(s) = default(1000.000s);
        double updateRate @unit(s) = default(5.000s);
        int convoyDirection = default(0);
    gates:
        input in;
        output out;
}
