
package convoy_architecture.nodes;
import simu5g.nodes.NR.NRUe;
import inet.mobility.static.StationaryMobility;
import convoy_architecture.apps.Tracker;
import convoy_architecture.stores.DtwinStore;

module Sensorstation extends NRUe
{
    parameters:
        @display("i=abstract/multicast;is=vs");
        //# Mobility
        mobility.typename = default("StationaryMobility");
        numApps = 1;
        app[0].typename = "Tracker";

        //# Network Layer specs
        *.routingTableModule = default(absPath(".ipv4.routingTable"));
        
        ipv4.configurator.typename = "HostAutoConfigurator";
        ipv4.configurator.interfaces = "cellular lo0";
        
    submodules:
         ObjectStore: DtwinStore
         {
             @display("p=750,72.479996,row,140");
         }
	connections:
	    at.out++ --> ObjectStore.in; 
}
