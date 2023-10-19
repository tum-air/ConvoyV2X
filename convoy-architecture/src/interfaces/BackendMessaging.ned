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

package convoy_architecture.interfaces;

//
// TODO auto-generated module
//
simple BackendMessaging
{
    parameters:
        @display("i=block/control");
        @class("convoy_architecture::BackendMessaging");
        string destinationModuleDtwin = default("backend_top");
        int destinationPortDtwin = default(31000);
        int destinationPortConvoyOrch = default(41000);
        int sizeConvoyOrchMsg @unit(B)= default(20B);
        int convoyDirection = default(0);
        int nStations = default(15);
    gates:
        input inUlDtwin;
        input inLlDtwin;
        output outUlDtwin;
        output outLlDtwin;
        input inUlConvoyOrch;
        input inLlConvoyOrch;
        output outUlConvoyOrch;
        output outLlConvoyOrch;
        output outMcsAgent;
}
