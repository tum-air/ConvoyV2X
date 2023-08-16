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

package convoy_architecture.controls;

//
// TODO auto-generated module
//
simple ConvoyControl
{
    parameters:
        @display("i=block/control");
        @class("convoy_architecture::ConvoyControl");
        int convoyDirection @mutable = default(0);
        double clusterBroadcastScanDuration @unit(s) = default(3s);
        double clusterBroadcastScanInterval @unit(s) = default(1s);
        double clusterInitLocalizationInterval @unit(s) = default(5.0s);
        double ccsBroadcastInterval @unit(s) = default(0.1s);
        double publisherListUpdateInterval @unit(s) = default(0.1s);
        double agentRunModeUpdateInterval @unit(s) = default(0.1s);
        int maxNClusters @mutable = default(6);
        int stationType = default(0);
        double minRSSIClusterManager @mutable = default(0.0);
    gates:
        input in;
        output out;
}
