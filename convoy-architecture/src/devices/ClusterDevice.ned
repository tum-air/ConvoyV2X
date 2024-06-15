
package convoy_architecture.devices;

import inet.networklayer.common.InterfaceTable;
import inet.mobility.contract.IMobility;
import inet.networklayer.contract.INetworkLayer;
import simu5g.stack.ICellularNic;
import simu5g.stack.NRNicEnb;
import simu5g.stack.NRNicUe;
import simu5g.common.cellInfo.CellInfo;
import inet.common.MessageDispatcher;
import inet.transportlayer.udp.Udp;
import inet.applications.udpapp.UdpSocketIo;

//
// Common cluster device
//
module ClusterDevice
{
    parameters:
        @networkNode();
        @display("bgb=700,600;b=1,1,rect,red,red");
        @class("convoy_architecture::ClusterDevice");
        bool isPrimary = default(true);
        bool isMaster = default(true);
        
        string nicType = default("NRNicEnb");
        string nodeType = default("GNODEB");
        int nrMasterId @mutable = default(0);
        int nrMacNodeId @mutable = default(0);
        int nrMacCellId @mutable = default(0);
        int masterId @mutable = default(0);
        int macNodeId @mutable = default(0);
        int macCellId @mutable = default(0);
        double txPower @unit(mw) = default(100mw);
        string mobilityType = default("StationaryMobility");
        string IPv4Type = default("Ipv4NetworkLayer");
        string extHostAddress = default("");
        int convoyID @mutable = default(-1);
        bool convoyFull @mutable = default(false);
        bool clusterFull @mutable = default(false);
        int convoyDirection @mutable = default(0);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        
    gates:
        input radioIn @directIn;
        input nrRadioIn @directIn;
        input fromAppPublisher;
        output toAppPublisher;
        input fromAppSubscriber;
        output toAppSubscriber;
        input fromAppManeuver;
        output toAppManeuver;
        input fromAppMemberControl;
        output toAppMemberControl;
        input fromAppMemberReport;
        output toAppMemberReport;
        
    submodules:
        interfaceTable: InterfaceTable {
                @display("p=100,525;is=s");
        }
        mobility: <mobilityType> like IMobility {
                @display("p=600,525;is=s");
        }
        ipv4: <IPv4Type> like INetworkLayer {
            parameters:
                @display("p=350,375;is=n");
                configurator.typename = "HostAutoConfigurator";
        }
        cellInfo: CellInfo if isMaster {
            parameters:
                @display("p=600,375;is=s");
        }
        cellularNic: <nicType> like ICellularNic {
                nodeType = parent.nodeType;
                convoyID = parent.convoyID;
                convoyFull = parent.convoyFull;
                clusterFull = parent.clusterFull;
                convoyDirection = parent.convoyDirection;
                @display("p=350,525,row,150");
        }
        dispatcher_nl: MessageDispatcher {
            parameters:
                @display("p=350,450;b=200,5,,,,1");
        }
        dispatcher_tl: MessageDispatcher {
            parameters:
                @display("p=350,300;b=200,5,,,,1");
        }
        udp: Udp {
            parameters:
                @display("p=350,225");
        }
        dispatcher_al: MessageDispatcher {
            parameters:
                @display("p=350,150;b=200,5,,,,1");
        }
        ioAppSubscriber: UdpSocketIo {
                @display("p=100,125;is=s");
                localPort = 21001;
                destPort = 21002;
        }
        ioAppPublisher: UdpSocketIo {
                @display("p=100,50;is=s");
                localPort = 21002;
                destPort = 21001;
        }
        ioAppManeuver: UdpSocketIo {
                @display("p=350,50;is=s");
                localPort = 21003;
                destPort = 21003;
        }
        ioAppMemberControl: UdpSocketIo {
                @display("p=600,125;is=s");
                localPort = 21004;
                destPort = 21004;
        }
        ioAppMemberReport: UdpSocketIo {
                @display("p=600,50;is=s");
                localPort = 21005;
                destPort = 21005;
        }
        
    connections allowunconnected:
        // physical layer
        cellularNic.radioIn <-- radioIn;
        cellularNic.nrRadioIn <-- nrRadioIn;
        
        // network layer
        ipv4.ifIn <-- dispatcher_nl.out++;
        ipv4.ifOut --> dispatcher_nl.in++;
        cellularNic.upperLayerOut --> dispatcher_nl.in++;
        cellularNic.upperLayerIn <-- dispatcher_nl.out++;
        
        // transport layer
        udp.ipOut --> dispatcher_tl.in++;
        udp.ipIn <-- dispatcher_tl.out++;
        dispatcher_tl.out++ --> ipv4.transportIn;
        dispatcher_tl.in++ <-- ipv4.transportOut;
        
        // application layer
        dispatcher_al.out++ --> udp.appIn;
        dispatcher_al.in++ <-- udp.appOut;
        ioAppSubscriber.socketOut --> dispatcher_al.in++;
        ioAppPublisher.socketOut --> dispatcher_al.in++;
        ioAppManeuver.socketOut --> dispatcher_al.in++;
        ioAppMemberControl.socketOut --> dispatcher_al.in++;
        ioAppMemberReport.socketOut --> dispatcher_al.in++;
        ioAppSubscriber.socketIn <-- dispatcher_al.out++;
        ioAppPublisher.socketIn <-- dispatcher_al.out++;
        ioAppManeuver.socketIn <-- dispatcher_al.out++;
        ioAppMemberControl.socketIn <-- dispatcher_al.out++;
        ioAppMemberReport.socketIn <-- dispatcher_al.out++;
        
        ioAppSubscriber.trafficOut --> toAppSubscriber;
        ioAppPublisher.trafficOut --> toAppPublisher;
        ioAppManeuver.trafficOut --> toAppManeuver;
        ioAppMemberControl.trafficOut --> toAppMemberControl;
        ioAppMemberReport.trafficOut --> toAppMemberReport;
        fromAppSubscriber --> ioAppSubscriber.trafficIn;
        fromAppPublisher --> ioAppPublisher.trafficIn;
        fromAppManeuver --> ioAppManeuver.trafficIn;
        fromAppMemberControl --> ioAppMemberControl.trafficIn;
        fromAppMemberReport --> ioAppMemberReport.trafficIn;
}
