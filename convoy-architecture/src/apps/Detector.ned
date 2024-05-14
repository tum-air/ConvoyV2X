
package convoy_architecture.apps;

simple Detector
{
    parameters:
        string stationID = default("rsu_001");
        string stationType = default("rsu");
        string fovTag = default("down");
        double fovLimitRange @unit(m) = default(300m);
        double fovLimitAngleMin @unit(rad) = default(0.00rad);
        double fovLimitAngleMax @unit(rad) = default(1.57rad);
        double startTime @unit(s) = default(90.000s);
        double stopTime @unit(s) = default(1000.000s);
        double updateRate @unit(s) = default(0.100s);
        string detectedClasses = default("Vehicle,vehicle,car,truck,trailer,bus,van,bicycle,motorcycle,pedestrian");
        @application;
        @display("i=misc/securitycamera;is=n");
        @class("convoy_architecture::Detector");
	gates:
	    output detectionsEgo;
}
