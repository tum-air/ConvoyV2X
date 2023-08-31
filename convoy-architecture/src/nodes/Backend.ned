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

module Backend extends TransportLayerNodeBase
{
    parameters:
        @display("i=device/server;bgb=2500,1200;is=s");
        @networkNode();
        hasUdp = true;
        hasTcp = false;
        hasIpv4 = true;
        numLoInterfaces = 0;
        numEthInterfaces = default(1);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
    submodules:
        bmsAgent: BackendMessaging {
            @display("p=375,20;is=s");
        }
        dtwinStore: DtwinStore {
            @display("p=250,20");
        }
        ioAppDtwin: UdpSocketIo {
            @display("p=375,80;is=s");
            localPort = 31000;
            destPort = 31000;
        }
        dispatcher_al: MessageDispatcher {
            parameters:
                @display("p=375,160;b=200,5,,,,1");
        }
    connections allowunconnected:
        // application layer
        dispatcher_al.out++ --> udp.appIn;
        dispatcher_al.in++ <-- udp.appOut;
        ioAppDtwin.socketOut --> dispatcher_al.in++;
        ioAppDtwin.socketIn <-- dispatcher_al.out++;
        bmsAgent.outLl --> ioAppDtwin.trafficIn;
        ioAppDtwin.trafficOut --> bmsAgent.inLl;
        bmsAgent.outUl --> dtwinStore.in;
}
