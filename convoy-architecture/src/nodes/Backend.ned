
package convoy_architecture.nodes;

import convoy_architecture.stores.DtwinStore;
import convoy_architecture.controls.RoutingControl;
import inet.node.base.TransportLayerNodeBase;
import inet.applications.udpapp.UdpSocketIo;
import inet.common.MessageDispatcher;
import convoy_architecture.apps.ConvoyOrchestration;
import convoy_architecture.stores.MemberStore;

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
        int stationType = default(2);
    submodules:
        dtwinStore: DtwinStore {
            @display("p=375,20");
        }
        appConvoyOrch: ConvoyOrchestration {
            @display("p=500,20;is=s");
            convoyDirection = parent.convoyDirection;
        }
        memberStore: MemberStore {
            @display("p=250,20");
        }
        routingControl: RoutingControl {
            @display("p=375,80;is=s");
            convoyDirection = parent.convoyDirection;
        }
        ioAppSubscriber: UdpSocketIo {
            @display("p=375,130;is=s");
            localPort = 21001;
                destPort = 21002;
        }
        ioAppMemberReport: UdpSocketIo {
            @display("p=250,100;is=s");
            localPort = 21005;
            destPort = 21005;
        }
        ioAppMemberControl: UdpSocketIo {
            @display("p=500,100;is=s");
            localPort = 21004;
            destPort = 21004;
        }
        dispatcher_al: MessageDispatcher {
            parameters:
                @display("p=375,170;b=200,5,,,,1");
        }

    connections allowunconnected:
        // application layer
        dispatcher_al.out++ --> udp.appIn;
        dispatcher_al.in++ <-- udp.appOut;
        ioAppSubscriber.socketOut --> dispatcher_al.in++;
        ioAppSubscriber.socketIn <-- dispatcher_al.out++;
        ioAppMemberReport.socketOut --> dispatcher_al.in++;
        ioAppMemberReport.socketIn <-- dispatcher_al.out++;        
        ioAppMemberControl.socketOut --> dispatcher_al.in++;
        ioAppMemberControl.socketIn <-- dispatcher_al.out++;
        
        ioAppSubscriber.trafficOut --> routingControl.inLlSubscriber;
        ioAppSubscriber.trafficOut --> routingControl.inLlSubscriber;
        ioAppMemberReport.trafficOut --> routingControl.inLlMemberReport;
        ioAppMemberReport.trafficIn <-- routingControl.outLlMemberReport;
        ioAppMemberControl.trafficOut --> routingControl.inLlMemberControl;
        ioAppMemberControl.trafficIn <-- routingControl.outLlMemberControl;
        
        routingControl.outUlSubscriber --> dtwinStore.in;
        routingControl.outUlMemberControl --> memberStore.in;
        routingControl.inUlMemberControl <-- appConvoyOrch.out;
}
