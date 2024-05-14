
package convoy_architecture.apps;

simple Tracker
{
    parameters:
        @application;
        @display("i=block/routing;is=n");
        @class("convoy_architecture::Tracker");

    gates:
        output dtwinCollective;
        input dtwinNeighbours;
        input detectionsEgo;
}
