## IM system

## Packet Config

### Client recieved packets:
|      Name      | B1 |   B2  | B3 to B510      |                                         Notes                                        |
|:--------------:|:--:|:-----:|-----------------|:------------------------------------------------------------------------------------:|
|     Message    |  0 | 0-255 | Message Content |                           B2 is userID followed by message                           |
| New Connection |  1 | 0-255 | Username        | B2 is userID followed by username given, clients should save this for later messages |

### Server recieved packets:
|   Name  | B1 |  B2  | B3 to B510      |                         Notes                        |
|:-------:|:--:|:----:|-----------------|:----------------------------------------------------:|
| Message |  0 | NULL | Message Content | B2 is filled by the Server when broadcast to clients |