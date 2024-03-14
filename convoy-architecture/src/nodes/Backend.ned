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

import convoy_architecture.stores.DtwinStore;
import convoy_architecture.interfaces.BackendMessaging;
import inet.node.base.TransportLayerNodeBase;
import inet.applications.udpapp.UdpSocketIo;
import inet.common.MessageDispatcher;
import convoy_architecture.apps.ConvoyOrchestration;
import convoy_architecture.stores.SubscriberStore;

module Backend extends TransportLayerNodeBase
{
    parameters:
        @display("i=device/server;bgb=1300,1200;is=s");
        @networkNode();
        int convoyDirection = default(0);
        hasUdp = true;
        hasTcp = false;
        hasIpv4 = true;
        numLoInterfaces = 0;
        numEthInterfaces = default(1);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        mobility.typename = default("StationaryMobility");
        string detectorStationType = default("rsu");
    submodules:
        bmsAgent: BackendMessaging {
            @display("p=375,80;is=s");
            convoyDirection = parent.convoyDirection;
        }
        dtwinStore: DtwinStore {
            @display("p=375,20");
        }
        ioAppDtwin: UdpSocketIo {
            @display("p=375,130;is=s");
            localPort = 31000;
            destPort = 31000;
        }
        dispatcher_al: MessageDispatcher {
            parameters:
                @display("p=375,170;b=200,5,,,,1");
        }
        ioAppConvoyOrch: UdpSocketIo {
            @display("p=500,130;is=s");
            localPort = 41000;
            destPort = 41000;
        }
        appConvoyOrch: ConvoyOrchestration {
            @display("p=500,20;is=s");
            convoyDirection = parent.convoyDirection;
        }
        ioAppDtwinSub: UdpSocketIo {
            @display("p=250,130;is=s");
            localPort = 51000;
            destPort = 51000;
        }
        subscriberStore: SubscriberStore {
            @display("p=250,20");
        }
    connections allowunconnected:
        // application layer
        dispatcher_al.out++ --> udp.appIn;
        dispatcher_al.in++ <-- udp.appOut;
        ioAppDtwin.socketOut --> dispatcher_al.in++;
        ioAppDtwin.socketIn <-- dispatcher_al.out++;
        bmsAgent.outLlDtwin --> ioAppDtwin.trafficIn;
        ioAppDtwin.trafficOut --> bmsAgent.inLlDtwin;
        bmsAgent.outUlDtwin --> dtwinStore.in;
        ioAppConvoyOrch.socketOut --> dispatcher_al.in++;
        ioAppConvoyOrch.socketIn <-- dispatcher_al.out++;
        appConvoyOrch.out --> bmsAgent.inUlConvoyOrch;
        bmsAgent.outLlConvoyOrch --> ioAppConvoyOrch.trafficIn;
        bmsAgent.outUlConvoyOrch --> appConvoyOrch.in;
        ioAppConvoyOrch.trafficOut --> bmsAgent.inLlConvoyOrch;
        
        ioAppDtwinSub.socketOut --> dispatcher_al.in++;
        ioAppDtwinSub.socketIn <-- dispatcher_al.out++;
        bmsAgent.outLlDtwinSub --> ioAppDtwinSub.trafficIn;
        ioAppDtwinSub.trafficOut --> bmsAgent.inLlDtwinSub;
        bmsAgent.outUlDtwinSub --> subscriberStore.in;
}
