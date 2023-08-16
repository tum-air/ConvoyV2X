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

import simu5g.stack.ICellularNic;
import inet.common.MessageDispatcher;
import convoy_architecture.controls.MessagingControl;
import inet.networklayer.common.InterfaceTable;
import inet.mobility.contract.IMobility;
import convoy_architecture.apps.DtwinSubscriber;
import convoy_architecture.apps.DtwinPublisher;
import convoy_architecture.apps.CooperativeManeuver;
import convoy_architecture.stores.DtwinStore;
import convoy_architecture.apps.Detector;
import convoy_architecture.apps.Tracker;
import convoy_architecture.apps.Localizer;
import convoy_architecture.controls.ConvoyControl;

module Generic
{
    parameters:
        @networkNode();
        @display("i=device/server;bgb=700,800");
        string nicType = default("NRNicUe");
        string nodeType = "UE";
        int nrMasterId @mutable = default(0);
        int nrMacNodeId @mutable = default(0);
        int nrMacCellId @mutable = default(0);
        int numNICs = default(2);
        mobility.typename = default("StationaryMobility");
        string stationType = default("rsu");
        bool hasDtwinSub = default(true);
        bool hasDtwinPub = default(true);
        bool hasCoopMan = default(false);
        bool hasTracker = default(true);
        volatile string stationID = default("rsu_001");
        volatile double stationPX @unit(m) = default(0m);
        volatile double stationPY @unit(m) = default(0m);
        volatile double stationPZ @unit(m) = default(0m);
        volatile double stationPH @unit(rad) = default(0rad);
        volatile double stationAge @unit(s) = default(0s);
        volatile bool stationLocalized = default(false);

    gates:
        input nrRadioIn[] @directIn;

    submodules:
        interfaceTable: InterfaceTable {
            @display("p=100,650;is=s");
        }
        mobility: <default("")> like IMobility if typename != "" {
            @display("p=600,650;is=s");
        }
        cellularNic[numNICs]: <nicType> like ICellularNic {
            nodeType = parent.nodeType;
            @display("p=350,650,row,150");
        }
        nl[numNICs]: MessageDispatcher {
            @display("p=350,500,row,150;b=200,5,,,,1");
        }
        mcsAgent: MessagingControl {
            //interfaceTableModule = absPath(".interfaceTable");
            @display("p=350,350");
        }
        appDtwinSub: DtwinSubscriber if hasDtwinSub {
            @display("p=350,200");
        }
        appDtwinPub: DtwinPublisher if hasDtwinPub {
            @display("p=100,350");
        }
        appCoopMan: CooperativeManeuver if hasCoopMan {
            @display("p=600,350");
        }
        appEgoDetector: Detector if hasTracker {
            @display("p=600,50");
        }
        appTracker: Tracker if hasTracker {
            @display("p=350,50");
        }
        dtwinStore: DtwinStore if hasTracker || hasDtwinSub {
            @display("p=100,50;is=n");
        }
        appLocalizer: Localizer if hasTracker || hasDtwinSub {
            @display("p=100,500;is=n");
        }
        ccsAgent: ConvoyControl if hasTracker || hasDtwinSub {
            @display("p=600,500;is=n");
        }

    connections allowunconnected:
        for i=0..numNICs-1 {
            cellularNic[i].nrRadioIn <-- nrRadioIn[i];
            cellularNic[i].upperLayerOut --> nl[i].in++;
        	cellularNic[i].upperLayerIn <-- nl[i].out++;
        	
//        	mcsAgent.ifOutNIC[i] --> nl[i].in++;
//        	mcsAgent.ifInNIC[i] <-- nl[i].out++;
        }

//        mcsAgent.ifOutAppDtwinSub --> appDtwinSub.in if hasDtwinSub;
//        mcsAgent.ifOutAppCoopMan --> appCoopMan.in if hasCoopMan;
//        mcsAgent.ifInAppDtwinSub <-- appDtwinSub.out_sub if hasDtwinSub;
//        mcsAgent.ifInAppDtwinPub <-- appDtwinPub.out if hasDtwinPub;
//        mcsAgent.ifInAppCoopMan <-- appCoopMan.out if hasCoopMan;

		appEgoDetector.detectionsEgo --> appTracker.detectionsEgo if hasTracker;
		appTracker.dtwinNeighbours <-- appDtwinSub.out_dtwin if hasTracker;
		
		dtwinStore.in <-- appTracker.dtwinCollective if hasTracker;
		dtwinStore.in <-- appDtwinSub.out_dtwin if (!hasTracker) && hasDtwinSub;
}
