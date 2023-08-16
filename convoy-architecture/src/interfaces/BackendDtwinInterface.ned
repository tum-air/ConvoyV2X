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

package convoy_architecture.interfaces;

import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.common.MessageDispatcher;
import inet.transportlayer.udp.Udp;
import inet.applications.udpapp.UdpSocketIo;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.contract.IEthernetInterface;

module BackendDtwinInterface
{
    parameters:
        @networkNode();
        @display("bgb=700,750;i=block/ifcard;is=s");
        string IPv4Type = default("Ipv4NetworkLayer");
        string extHostAddress = default("");
        string hostConfigurator = default("HostAutoConfigurator");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
    gates:
        inout ethg;
        input fromAppDtwinPub;
        output toAppDtwinPub;
    submodules:
        interfaceTable: InterfaceTable {
                @display("p=100,525;is=s");
        }
        ipv4: <IPv4Type> like INetworkLayer {
            parameters:
                @display("p=350,375;is=n");
                configurator.typename = parent.hostConfigurator;
                configurator.interfaces ="eth";
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
        ioAppDtwinPub: UdpSocketIo {
                @display("p=350,75;is=s");
                localPort = 31001;
                destPort = 31001;
        }
        ethernet: <"EthernetEncapsulation"> like IEthernetLayer {
            @display("p=350,525");
        }
        dispatcher_ll: MessageDispatcher {
            parameters:
                @display("p=350,600;b=200,5,,,,1");
        }
        eth: <"EthernetInterface"> like IEthernetInterface {
            bitrate = 10Gbps;
            @display("p=350,675");
        }
    connections allowunconnected:
        // ethernet layer
        ethg <--> { @display("m=s"); } <--> eth.phys;
        eth.upperLayerOut --> dispatcher_ll.in++;
        eth.upperLayerIn <-- dispatcher_ll.out++;
        ethernet.lowerLayerOut --> dispatcher_ll.in++;
        dispatcher_ll.out++ --> ethernet.lowerLayerIn;
        
        // network layer
        ipv4.ifIn <-- dispatcher_nl.out++;
        ipv4.ifOut --> dispatcher_nl.in++;
        dispatcher_nl.out++ --> ethernet.upperLayerIn;
        ethernet.upperLayerOut --> dispatcher_nl.in++;
        
        // transport layer
        udp.ipOut --> dispatcher_tl.in++;
        udp.ipIn <-- dispatcher_tl.out++;
        dispatcher_tl.out++ --> ipv4.transportIn;
        dispatcher_tl.in++ <-- ipv4.transportOut;
        
        // application layer
        dispatcher_al.out++ --> udp.appIn;
        dispatcher_al.in++ <-- udp.appOut;

        ioAppDtwinPub.socketOut --> dispatcher_al.in++;
        ioAppDtwinPub.trafficOut --> toAppDtwinPub;
        fromAppDtwinPub --> ioAppDtwinPub.trafficIn;
}
