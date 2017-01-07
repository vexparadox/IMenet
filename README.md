## IM system

## Packet Config

### Server sent packets:
|      Name      | B1 |   B2  | B3 to B510      |                                         Notes                                        |
|:--------------:|:--:|:-----:|-----------------|:------------------------------------------------------------------------------------:|
|     Message    |  0 | 0-255 | Message Content | B2 is userID followed by message.        |
| New username |  1 | 0-255 | Username        | B2 is userID followed by username given. |

### Client sent packets:
Note that B2 is used for UserIDs, the Client doesn't need to know about its own ID. This will be filled by the server. Leave B2 empty for clarity, it's ignored by the Server anyway.
|   Name  | B1 |  B2  | B3 to B510      |                         Notes                        |
|:-------:|:--:|:----:|-----------------|:----------------------------------------------------:|
| Message |  0 | NULL | Message Content | B2 is filled by the Server when broadcast to clients |
| Username |  1 | NULL | Username | B3-B510 is saved and broadcast to other clients |

### TODO
Implement the Client Sent Username packet

Make sure new clients are told about the older clients