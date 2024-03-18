
package convoy_architecture.apps;

//
// Localizer application for the vehicles
// Runs for a short duration after initialization for the
// vehicle to identify itself in the received digital twin
//

simple Localizer
{
    parameters:
        string stationType = default("rsu");
        double startTime @unit(s) = default(90.000s);
        double stopTime @unit(s) = default(1000.000s);
        double updateRate @unit(s) = default(0.100s);
        double minDurationLocalization @unit(s) = default(2.000s);
        double maxAgeLocalization @unit(s) = default(2.0s);
        string strategyLocalization = default("VISUAL_CUE");
        @application;
        @display("i=block/routing");
        @class("convoy_architecture::Localizer");
}