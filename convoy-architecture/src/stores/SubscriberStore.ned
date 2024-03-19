
package convoy_architecture.stores;

//
// Container to store the list of active subscribers 
//

simple SubscriberStore
{
    parameters:
        double subscriberMaxAge @unit(s) = default(5.000s);
        double subscriberCheckInterval @unit(s) = default(1.000s);

        @display("i=block/buffer2;is=s");
        @class("convoy_architecture::SubscriberStore");
    gates:
        input in;
}
