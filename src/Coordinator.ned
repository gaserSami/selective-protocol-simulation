package selective_protocol_simulation;

simple Coordinator
{
    @display("i=block/cogwheel");
    
    gates:
        input gin[2];       // Input gate of size: 2
        output gout[2];     // Output gate of size: 2
}
