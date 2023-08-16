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
simple MessagingControl
{
    parameters:
        @display("i=block/control");
        @class("convoy_architecture::MessagingControl");
        int convoyDirection @mutable = default(0);
        double roiHorizon @unit(m) = default(1000.0m);
        double roiNearby @unit(m) = default(300.0m);
        int sizeDtwinSubMsg @unit(B)= default(10B);
        int sizeCoopManMsg @unit(B)= default(20B);
        int destPortAppDtwinPub = default(21001);
        int destPortAppDtwinSub = default(21002);
        int destPortAppCoopMan = default(21003);
        double subscriberExpiryCheckInterval @unit(s) = default(1.0s);
        double subscriberMaxAge @unit(s) = default(5.0s);

	gates:
        output outLlAppDtwinPub;
        input inLlAppDtwinPub;
        output outLlAppDtwinSub;
        input inLlAppDtwinSub;
        output outLlAppCoopMan;
        input inLlAppCoopMan;
        output outUlAppDtwinPub;
        input inUlAppDtwinPub;
        output outUlAppDtwinSub;
        input inUlAppDtwinSub;
        output outUlAppCoopMan;
        input inUlAppCoopMan;
}
