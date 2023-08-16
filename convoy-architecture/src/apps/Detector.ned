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
simple Detector
{
    parameters:
        string stationID = default("rsu_001");
        string stationType = default("rsu");
        string fovTag = default("down");
        double fovLimitRange @unit(m) = default(300m);
        double fovLimitAngleMin @unit(rad) = default(0.00rad);
        double fovLimitAngleMax @unit(rad) = default(1.57rad);
        double startTime @unit(s) = default(90.000s);
        double stopTime @unit(s) = default(1000.000s);
        double updateRate @unit(s) = default(0.100s);
        string detectedClasses = default("Vehicle,vehicle,car,truck,trailer,bus,van,bicycle,motorcycle,pedestrian");
        @application;
        @display("i=misc/securitycamera;is=n");
        @class("convoy_architecture::Detector");
	gates:
	    output detectionsEgo;
}
