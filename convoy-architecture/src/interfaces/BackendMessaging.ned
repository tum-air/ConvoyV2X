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
        string destinationModule = default("backend_interface_top");
        int destinationPort = default(31001);
    gates:
        input inUl;
        input inLl;
        output outUl;
        output outLl;
}
