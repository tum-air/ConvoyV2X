
package convoy_architecture.controls;

//
// Membership control agent
//
simple MembershipControl
{
    parameters:
        @display("i=block/control");
        @class("convoy_architecture::MembershipControl");
        double broadcastScanDuration @unit(s) = default(3s);
        double broadcastScanInterval @unit(s) = default(1s);
        double minRSSIClusterManager @mutable = default(0.0);
        int convoyDirection = parent.convoyDirection;
        double publisherUpdateInterval @unit(s) = default(0.1s);
        double memberReportStartTime @unit(s) = default(750.000s);
        double memberReportUpdateRate @unit(s) = default(1s);
    gates:
        input in;
        output out;
}
