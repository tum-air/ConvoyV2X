//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package convoy_architecture.nodes;

import inet.networklayer.common.InterfaceTable;
import inet.mobility.contract.IMobility;
import inet.networklayer.contract.INetworkLayer;
import simu5g.stack.NRNicEnb;
import simu5g.common.cellInfo.CellInfo;
import inet.common.MessageDispatcher;
import inet.transportlayer.udp.Udp;
import inet.applications.udpapp.UdpSocketIo;

module ClusterManager
{
    parameters:
        @networkNode();
        @display("bgb=700,600;b=1,1,rect,red,red");
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
        volatile int convoyID @mutable = default(-1);
        volatile bool convoyFull @mutable = default(false);
        volatile bool clusterFull @mutable = default(false);
        volatile int convoyDirection @mutable = default(0);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));

	gates:
        input radioIn @directIn;
        input fromAppDtwinPub;
        output toAppDtwinPub;
        input fromAppDtwinSub;
        output toAppDtwinSub;
        input fromAppCoopMan;
        output toAppCoopMan;

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
        cellInfo: CellInfo {
            parameters:
                @display("p=600,375;is=s");
        }
        cellularNic: NRNicEnb {
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
        ioAppDtwinSub: UdpSocketIo {
            @display("p=150,75;is=s");
            localPort = 21001;
            destPort = 21002;
        }
        ioAppDtwinPub: UdpSocketIo {
            @display("p=350,75;is=s");
            localPort = 21002;
            destPort = 21001;
        }
        ioAppCoopMan: UdpSocketIo {
            @display("p=550,75;is=s");
            localPort = 21003;
            destPort = 21003;
        }

    connections allowunconnected:
        // physical layer
        cellularNic.radioIn <-- radioIn;
        
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
        ioAppDtwinSub.socketOut --> dispatcher_al.in++;
		ioAppDtwinPub.socketOut --> dispatcher_al.in++;
		ioAppCoopMan.socketOut --> dispatcher_al.in++;
		ioAppDtwinSub.socketIn <-- dispatcher_al.out++;
		ioAppDtwinPub.socketIn <-- dispatcher_al.out++;
		ioAppCoopMan.socketIn <-- dispatcher_al.out++;
		
		ioAppDtwinSub.trafficOut --> toAppDtwinSub;
        ioAppDtwinPub.trafficOut --> toAppDtwinPub;
        ioAppCoopMan.trafficOut --> toAppCoopMan;
        fromAppDtwinSub --> ioAppDtwinSub.trafficIn;
        fromAppDtwinPub --> ioAppDtwinPub.trafficIn;
        fromAppCoopMan --> ioAppCoopMan.trafficIn;
}
