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
import simu5g.stack.NRNicUe;
import simu5g.stack.NRNicEnb;
import simu5g.common.cellInfo.CellInfo;
import convoy_architecture.controls.ConvoyControl;
import inet.common.MessageDispatcher;

module Station
{
    parameters:
        @networkNode();
        @display("i=device/server;bgb=700,800;is=s");
        volatile string stationType @mutable = default("rsu");
        volatile string stationID @mutable = default("rsu_001");
        string extHostAddress = default("");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default("^.ipv4.routingTable");
        bool hasDtwinSub = default(true);
        bool hasDtwinPub = default(true);
        bool hasCoopMan = default(false);
        bool hasTracker = default(true);
        volatile string nicType @mutable = default("NRNicUe");
        volatile string enbType @mutable = default("NRNicEnb");
        volatile int nrMasterId @mutable = default(0);
        volatile int nrMacNodeId @mutable = default(0);
        volatile int nrMacCellId @mutable = default(0);
        volatile int masterId @mutable = default(0);
        volatile int macNodeId @mutable = default(0);
        volatile int macCellId @mutable = default(0);
        double txPower @unit(mw) = default(100mw);

	gates:
        input nrRadioIn[3] @directIn;

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=600,725;is=s");
        }
        mobility: <default("")> like IMobility if typename != "" {
            @display("p=600,650;is=s");
        }
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer {
            parameters:
                configurator.interfaces = "cellular";
                configurator.typename = "HostAutoConfigurator";
                @display("p=350,350;is=n");
        }
        cellInfo: CellInfo {
            parameters:
                @display("p=100,650;is=s");
        }
        manager: NRNicEnb {
            nodeType = "GNODEB";
            @display("p=250,650,row,150");
        }
        
        //member: NRNicUe {
        //    nodeType = "UE";
        //    @display("p=350,650,row,150");
        //}
        
        gateway: NRNicUe {
            nodeType = "UE";
            @display("p=450,650,row,150");
        }
        ccsAgent: ConvoyControl if hasTracker || hasDtwinSub {
            @display("p=600,500;is=n");
        }
        dispatcher_nl: MessageDispatcher {
            parameters:
                @display("p=350,500;b=200,5,,,,1");
        }

    connections allowunconnected:
        // physical layer
        manager.nrRadioIn <-- nrRadioIn[0];
        //member.nrRadioIn <-- nrRadioIn[1];
        gateway.nrRadioIn <-- nrRadioIn[2];
        
        // network layer
        ipv4.ifIn <-- dispatcher_nl.out++;
        ipv4.ifOut --> dispatcher_nl.in++;
        manager.upperLayerOut --> dispatcher_nl.in++;
        manager.upperLayerIn <-- dispatcher_nl.out++;
        //member.upperLayerOut --> dispatcher_nl.in++;
        //member.upperLayerIn <-- dispatcher_nl.out++;
        gateway.upperLayerOut --> dispatcher_nl.in++;
        gateway.upperLayerIn <-- dispatcher_nl.out++;
}
