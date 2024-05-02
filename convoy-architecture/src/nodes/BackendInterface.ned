
package convoy_architecture.nodes;

import inet.node.base.TransportLayerNodeBase;
import inet.applications.udpapp.UdpSocketIo;
import inet.common.MessageDispatcher;

module BackendInterface extends TransportLayerNodeBase
{
    parameters:
        @display("i=block/ifcard;bgb=2500,1200;is=vs");
        @networkNode();
        hasUdp = true;
        hasTcp = false;
        hasIpv4 = true;
        forwarding = true;
        numLoInterfaces = 0;
        numEthInterfaces = default(1);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
    gates:
        input inBkndPublisher;
        input inBkndMemberReport;
        output outBkndMemberControl;
    submodules:
        ioAppPublisher: UdpSocketIo {
            @display("p=275,100;is=s");
            localPort = 21002;
            destPort = 21001;
        }
        ioAppMemberReport: UdpSocketIo {
                @display("p=375,30;is=s");
                localPort = 21005;
                destPort = 21005;
        }
        ioAppMemberControl: UdpSocketIo {
            @display("p=475,100;is=s");
            localPort = 21004;
            destPort = 21004;
        }
        dispatcher_al: MessageDispatcher {
            parameters:
                @display("p=375,160;b=200,5,,,,1");
        }
    connections allowunconnected:
        // application layer
        dispatcher_al.out++ --> udp.appIn;
        dispatcher_al.in++ <-- udp.appOut;
        ioAppPublisher.socketOut --> dispatcher_al.in++;
        ioAppPublisher.socketIn <-- dispatcher_al.out++;
        ioAppMemberReport.socketOut --> dispatcher_al.in++;
        ioAppMemberReport.socketIn <-- dispatcher_al.out++;
        ioAppMemberControl.socketOut --> dispatcher_al.in++;
        ioAppMemberControl.socketIn <-- dispatcher_al.out++;
        inBkndPublisher --> ioAppPublisher.trafficIn;
        inBkndMemberReport --> ioAppMemberReport.trafficIn;
        outBkndMemberControl <-- ioAppMemberControl.trafficOut;
}
