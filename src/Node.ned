// Node Module NED file
package selective_protocol_simulation;


simple Node
{
    parameters:
        int WS = default(3); 			// Sender window Size
        int WR = default(3); 			// receiver window size
        double PT = default(0.5); 		// Sender and receiver frame proccessing time
        double TO = default(10.0); 		// Sender timout
        double TD = default(1.0); 	    // Channel transmission delay
        double ED = default(4.0); 	    // Channel error delay 
        double DD = default(0.1); 	    // Channel dulpication delay
        @display("i=device/device");
        
    gates:
        input gin[2];				// Input gate of size: 2
        output gout[2];			   // Output gate of size: 2
}
