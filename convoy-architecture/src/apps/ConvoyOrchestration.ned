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
simple ConvoyOrchestration
{
    parameters:
        @application;
        @display("i=block/network2;is=s");
        @class("convoy_architecture::ConvoyOrchestration");
        double startTime @unit(s) = default(70.000s);
        double stopTime @unit(s) = default(1000.000s);
        double updateRate @unit(s) = default(5.000s);
        int convoyDirection = default(0);
    gates:
        input in;
        output out;
}
