cplusplus {{
#include <string>
#include <bitset>
typedef  std::bitset<8> byte;
}}
class byte { @existingClass; }



packet Frame {
    @customize(true); 	 			// see the generated C++ header for more info
   	int dataSeqNr;	 					// Data sequence number
   	int type; 								// Data=0, ACK=1, NACK=2.
   	int ackSeqNr;  						// ACK/NACK number
   	string payload; 					// The message contents after byte stuffing (in char, including flags)
   	byte trailer; 						// Trailer: Checksum
}
