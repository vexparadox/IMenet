# IMenet
This is a micro instant messaging system, there's a server and client side. Users can have a custom Username and talk to others on the same server.

**This project was started in University over 6 years ago and I updating for the sake of curiosity.**

## Requirements & Setup
If you're compiling from scratch you'll need the Enet library to be linked, http://enet.bespin.org. On Linux and OSX it's a simple make install process, it's also available on HomeBrew `brew install enet`.

If you're using the pre-compiled OSX Standalone, skip to the second step

* To build on OSX and Linux use the makefile provided.

* Start using `./output ip port`

* Make sure the server has started before the client. 

NB - If the server/client fails to start it could be caused by you using an ip/port that's not available.


## Packet Config
Each packet is 512 bytes, BN means byte N of 512 in a packet.

B1 is the ID of the packets action, this can be used in a switch or array of functions.
### Server to Client packets:
|      Name      | B1 |   B2  | B3 to B512      |                                         Notes                                        |
|:--------------:|:--:|:-----:|-----------------|:------------------------------------------------------------------------------------:|
|     Message    |  0 | 0-255 | Message Content | B2 is userID followed by message.        |
| New username |  1 | 0-255 | Username        | B2 is userID followed by username given. |
| User disconnected |  2 | 0-255 | NULL        | B2 has disconnected |

### Client to Server packets:
Note that B2 is used for UserIDs, the Client doesn't need to know about its own ID. This will be filled by the server. Leave B2 empty for clarity, it's ignored by the Server anyway.

|   Name  | B1 |  B2  | B3 to B512      |                         Notes                        |
|:-------:|:--:|:----:|-----------------|:----------------------------------------------------:|
| Message |  0 | NULL | Message Content | B2 is filled by the Server when broadcast to clients |
| Username |  1 | NULL | Username | B3-B510 is saved and broadcast to other clients |

### One day...
OpenSSL support

Add a server disconnect packet, S->C. Could use the User Disconnected packet, special case for 255 (Server ID)