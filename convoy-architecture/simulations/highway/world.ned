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

package convoy_architecture.simulations.highway;

import convoy_architecture.world.test.TestWorld;
import convoy_architecture.nodes.WrapperNode;
import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;
import simu5g.world.radio.LteChannelControl;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import convoy_architecture.interfaces.BackendDtwinInterface;

network world
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        int n_stations = 15;
        @display("bgb=5000,1500,DarkSeaGreen3,grey75");

    submodules:
        // Base scenario module
        scenarioConfig: TestWorld {
            @display("p=300,1350;is=s");
        }
        //# Veins manager module
        veinsManager: VeinsInetManager {
            @display("p=1875,1350;is=s");
        }
        //# LTE modules
        channelControl: LteChannelControl {
            @display("p=2500,1350;is=s");
        }
        binder: Binder {
            @display("p=3125,1350;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=3750,1350;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=625,1350;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=1250,1350;is=s");
            config = xmldoc("ip_config.xml");
        }
        
//        backend_interface_top: BackendDtwinInterface {
//            @display("p=2350,130;is=s");
//        }
//        backend_interface_bot: BackendDtwinInterface {
//            @display("p=2350,320;is=s");
//        }
//        st_bknd_intf_top[15]: BackendDtwinInterface {
//            @display("p=405,150,r,300");
//        }
//        st_bknd_intf_bot[15]: BackendDtwinInterface {
//            @display("p=405,300,r,300");
//        }
//        
//    connections allowunconnected:
}
