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
import convoy_architecture.interfaces.MultipleInputSingleOutput;

module Backend
{
    parameters:
        @display("i=device/server;bgb=300,300;is=s");
        int n_stations = default(15);
    gates:
        input inAppDtwinPub[n_stations];
    submodules:
        ipModule: MultipleInputSingleOutput {
            nInputGates = parent.n_stations;
            @display("p=150,250");
        }
        bmsAgent: BackendMessaging {
            @display("p=250,150");
        }
        dtwinStore: DtwinStore {
            @display("p=150,50");
        }
    connections allowunconnected:
        for i=0..n_stations-1
        {
        	inAppDtwinPub[i] --> ipModule.in[i];
        }
        ipModule.out --> dtwinStore.in;
}