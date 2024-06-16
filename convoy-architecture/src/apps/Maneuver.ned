
package convoy_architecture.apps;

//
// Application for cooperative maneuvering
//

simple Maneuver
{
    parameters:
        string egoID = default("rsu_001");
        string roiTag = default("neighborhood");
        string qosTag = default("ASIL-D");
        double startTime @unit(s) = default(100.000s);
        double stopTime @unit(s) = default(1000.000s);
        double updateRate @unit(s) = default(0.100s);
        double limitROINeighborhood @unit(m) = default(100m);
        double limitROIDefault @unit(m) = default(300m);
        double limitROIHorizon @unit(m) = default(1000m);
        @application;
        @display("i=block/source");
        @class("convoy_architecture::Maneuver");
  	gates:
  	    input in;
	    output out;
}