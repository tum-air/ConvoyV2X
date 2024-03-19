
package convoy_architecture.stores;

//
// Container to store the membership status of active nodes 
//

simple MemberStore
{
    parameters:
        double memberMaxAge @unit(s) = default(5.000s);
        double memberCheckInterval @unit(s) = default(1.000s);

        @display("i=block/buffer2;is=s");
        @class("convoy_architecture::MemberStore");
    gates:
        input in;
}