
package convoy_architecture.simulations.highway;

import convoy_architecture.scenarios.highway.HighwayScenario;
import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;
import simu5g.world.radio.LteChannelControl;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import convoy_architecture.nodes.Backend;
import convoy_architecture.nodes.BackendInterface;
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
        scenarioConfig: HighwayScenario {
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
        
        backendTop: Backend {
            @display("p=2350,110;is=s");
            convoyDirection = 1;
        }
        backendBot: Backend {
            @display("p=2350,340;is=s");
            convoyDirection = 2;
        }
        
        switchTop: EthernetSwitch {
            numEthInterfaces = parent.n_stations + 1;
            @display("p=2350,130;is=s");
        }
        switchBot: EthernetSwitch {
            numEthInterfaces = parent.n_stations + 1;
            @display("p=2350,320;is=s");
        }
        
        backendInterfaceTop[n_stations]: BackendInterface {
            @display("p=405,130,row,300;is=vs");
        }
        
        backendInterfaceBot[n_stations]: BackendInterface {
            @display("p=405,320,row,300;is=vs");
        }
        
    connections allowunconnected:
        backendTop.ethg[0] <--> Eth10G <--> switchTop.ethg[0];
        backendBot.ethg[0] <--> Eth10G <--> switchBot.ethg[0];
        
        for i=0..(n_stations-1) {
            backendInterfaceTop[i].ethg[0] <--> Eth10G <--> switchTop.ethg[i+1];
            backendInterfaceBot[i].ethg[0] <--> Eth10G <--> switchBot.ethg[i+1];
        }
}
