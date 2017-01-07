# IM system

## Requirements & Setup
Requires Enet to be linked, http://enet.bespin.org. On Linux and OSX it's a simple make install process. The makefiles included will link to a shared library, for Windows this may have to change.

To build on OSX and Linux use the makefile provided.

Start using './output ip port'

Make sure the server has started before the client. 

If the server/client fails to start it could be caused by you using an ip/port that's not available.


## Packet Config
B1 is the ID of the packet, this can be used in a switch.
### Server to Client packets:
|      Name      | B1 |   B2  | B3 to B510      |                                         Notes                                        |
|:--------------:|:--:|:-----:|-----------------|:------------------------------------------------------------------------------------:|
|     Message    |  0 | 0-255 | Message Content | B2 is userID followed by message.        |
| New username |  1 | 0-255 | Username        | B2 is userID followed by username given. |
| User disconnected |  2 | 0-255 | NULL        | B2 has disconnected |

### Client to Server packets:
Note that B2 is used for UserIDs, the Client doesn't need to know about its own ID. This will be filled by the server. Leave B2 empty for clarity, it's ignored by the Server anyway.

|   Name  | B1 |  B2  | B3 to B510      |                         Notes                        |
|:-------:|:--:|:----:|-----------------|:----------------------------------------------------:|
| Message |  0 | NULL | Message Content | B2 is filled by the Server when broadcast to clients |
| Username |  1 | NULL | Username | B3-B510 is saved and broadcast to other clients |

### TODO