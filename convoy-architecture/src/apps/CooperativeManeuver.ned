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

package convoy_architecture.apps;

//
// TODO auto-generated module
//
simple CooperativeManeuver
{
    parameters:
        volatile string egoID = default("rsu_001");
        volatile string roiTag = default("neighborhood");
        volatile string qosTag = default("ASIL-D");
        volatile double startTime @unit(s) = default(100.000s);
        volatile double stopTime @unit(s) = default(1000.000s);
        volatile double updateRate @unit(s) = default(0.100s);
        double limitROINeighborhood @unit(m) = default(100m);
        double limitROIDefault @unit(m) = default(300m);
        double limitROIHorizon @unit(m) = default(1000m);
        @application;
        @display("i=block/source");
        @class("convoy_architecture::CooperativeManeuver");
  	gates:
  	    input in;
	    output out;
}
