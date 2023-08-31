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
import convoy_architecture.nodes.Backend;
import convoy_architecture.interfaces.BackendDtwinInterface;
import inet.node.ethernet.EthernetSwitch;
import inet.node.ethernet.Eth10G;

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
        
        backend_top: Backend {
            @display("p=2350,110;is=s");
        }
        backend_bot: Backend {
            @display("p=2350,340;is=s");
        }
        
        switch_top: EthernetSwitch {
            numEthInterfaces = parent.n_stations + 1;
            @display("p=2350,130;is=s");
        }
        switch_bot: EthernetSwitch {
            numEthInterfaces = parent.n_stations + 1;
            @display("p=2350,320;is=s");
        }
        
        bk_if_top[n_stations]: BackendDtwinInterface {
            @display("p=405,130,row,300;is=vs");
        }
        
        bk_if_bot[n_stations]: BackendDtwinInterface {
            @display("p=405,320,row,300;is=vs");
        }
        
    connections allowunconnected:
        backend_top.ethg[0] <--> Eth10G <--> switch_top.ethg[0];
        backend_bot.ethg[0] <--> Eth10G <--> switch_bot.ethg[0];
        
        for i=0..(n_stations-1) {
            bk_if_top[i].ethg[0] <--> Eth10G <--> switch_top.ethg[i+1];
            bk_if_bot[i].ethg[0] <--> Eth10G <--> switch_bot.ethg[i+1];
        }
}
