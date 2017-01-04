## IM system

## Packet Config

### Client recieved by packets:
|      Name      | B1 |   B2  | B3 to B510      |                                         Notes                                        |
|:--------------:|:--:|:-----:|-----------------|:------------------------------------------------------------------------------------:|
|     Message    |  0 | 0-254 | Message Content |                           B2 is userID followed by message                           |
| New Connection |  1 | 0-254 | Username        | B2 is userID followed by username given, clients should save this for later messages |
|                |    |       |                 |                                                                                      |