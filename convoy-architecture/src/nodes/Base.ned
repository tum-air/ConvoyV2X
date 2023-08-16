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
import simu5g.stack.ICellularNic;
import simu5g.common.cellInfo.CellInfo;
import convoy_architecture.controls.ConvoyControl;
import inet.common.MessageDispatcher;

module Base
{
    parameters:
        @networkNode();
        @display("i=device/server;bgb=700,800;is=s");
        volatile string nicType @mutable = default("NRNicUe");
        volatile string nodeType @mutable = "UE";
        volatile int nrMasterId @mutable = default(0);
        volatile int nrMacNodeId @mutable = default(0);
        volatile int nrMacCellId @mutable = default(0);
        volatile int masterId @mutable = default(0);
        volatile int macNodeId @mutable = default(0);
        volatile int macCellId @mutable = default(0);
        mobility.typename = default("StationaryMobility");
        volatile string stationType @mutable = default("rsu");
        volatile string stationID @mutable = default("rsu_001");
        string extHostAddress = default("");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default("^.ipv4.routingTable");
        bool hasDtwinSub = default(true);
        bool hasDtwinPub = default(true);
        bool hasCoopMan = default(false);
        bool hasTracker = default(true);
        volatile bool isClusterManager @mutable = default(false);
        volatile bool isClusterGateway @mutable = default(false);
        double txPower @unit(mw) = default(100mw);

    gates:
        input nrRadioIn[2] @directIn;

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
        cellInfo: CellInfo if isClusterManager {
            parameters:
                @display("p=100,650;is=s");
        }
        cluster_member: <nicType> like ICellularNic {
            nodeType = parent.nodeType;
            @display("p=300,650,row,150");
        }
        gateway: <nicType> like ICellularNic if isClusterGateway {
            nodeType = parent.nodeType;
            @display("p=400,650,row,150");
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
        cluster_member.nrRadioIn <-- nrRadioIn[0];
        gateway.nrRadioIn <-- nrRadioIn[1] if isClusterGateway;
        
        // network layer
        ipv4.ifIn <-- dispatcher_nl.out++;
        ipv4.ifOut --> dispatcher_nl.in++;
        cluster_member.upperLayerOut --> dispatcher_nl.in++;
        cluster_member.upperLayerIn <-- dispatcher_nl.out++;
}