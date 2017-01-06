#include "main.hpp"
int main(int argc, char const *argv[])
{
    if (enet_initialize () != 0)
    {
        fprintf (stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit (enet_deinitialize);

    if(argc < 3){
    	fprintf(stderr, "No address or port was supplied\n");
    	return EXIT_FAILURE;
    }
	clientCount = 0;
	broadcastMessage = (char*)malloc(sizeof(char)*512);
    enet_address_set_host(&serverAddress, argv[1]);
    serverAddress.port = atoi(argv[2]);

    host.store(enet_host_create (&serverAddress, 32, 2, 0, 0));
    if (!host.load()) { printf("%s\n", "An error occurred while trying to create the server host."); }
    run.store(true);
    printf("Server was started on %s:%s, now listening for clients.\n", argv[1], argv[2]);
    memset(usernamesGiven, 0, 512);
    std::thread inputThread(&takeInput);
    while(run.load()){
        ENetEvent event;
        //wait upto half a second for an event
        while (enet_host_service (host.load(), &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:{
                    memset(broadcastMessage, 0, 512);
			        printf ("A new client connected from %x:%u.\n", 
			                event.peer -> address.host,
			                event.peer -> address.port);
			        /* Store any relevant client information here. */
					event.peer-> data = malloc(sizeof(char));
					*(char*)event.peer->data = clientCount;
					printf("%x\n", (char*)event.peer->data);
					clientCount++;
					char* welcome = "Welcome to the Server, please supply a username.";
                    memcpy(broadcastMessage+2, welcome, strlen(welcome));
                    broadcastMessage[0] = 0;
                    broadcastMessage[1] = 255;
	        	    ENetPacket* packet = enet_packet_create (broadcastMessage, 512, ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send (event.peer, 0, packet);
				    enet_host_flush (host.load());
                }break;
            case ENET_EVENT_TYPE_RECEIVE:{
                memset(broadcastMessage, 0, 512);
                //the server only listens to messages given aka[0]
                if(event.packet->data[0] == 0){
                    //if the user hasn't given a username yet
                    if(!usernamesGiven[*(char*)event.peer->data]){
                        broadcastMessage[0] = 1;
                        memcpy(broadcastMessage+1, event.peer->data, sizeof(char));
                        memcpy(broadcastMessage+2, event.packet->data+2, 510);
                        usernamesGiven[*(char*)event.peer->data] = 1;
                        printf("New user with name: %s\n", event.packet->data+2);
                    }else{
                        //if they ahve, just rebroadcast the message
        		       	//send out the data to everyone else
        	       		memcpy(broadcastMessage, event.peer->data, sizeof(char)); // stick the id at the start[]
        		       	memcpy(broadcastMessage+1, event.packet->data+2, 510); // copy the data
        		       	broadcastMessage[event.packet->dataLength+1] = '\0';

        		        printf ("%x: %s \n",
        		                broadcastMessage[1],
        		                broadcastMessage+2
        		                );
                    }
            	    ENetPacket* packet = enet_packet_create (broadcastMessage, 512, ENET_PACKET_FLAG_RELIABLE);
    			    enet_host_broadcast (host.load(), 0, packet);
    			    enet_host_flush (host.load());
                    enet_packet_destroy (event.packet);
                }
            }break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconnected.\n", event.peer -> data);
                event.peer -> data = NULL;
                break;
            case ENET_EVENT_TYPE_NONE:

                break;
            }
        }

    }

    enet_host_destroy(host);
    inputThread.join();
	/* code */
	return 0;
}

void takeInput(){
    // std::string buffer;
    //lets create a packet
    //512 bytes, 255 usable with 1 for ID
    char buffer[512];
    while (run.load()){
        fgets(buffer+2, sizeof(buffer), stdin);
        //get rid of that pesky \n
        char* temp = buffer+strlen(buffer)-1;
        *temp = '\0';
        buffer[0] = 0; // set the ID to 255, this is reserved for the server
        buffer[1] = 255; // set the ID to 255, this is reserved for the server
        if(strcmp(buffer, "") != 0){
            // controller->takeInput(buffer);
            if(strcmp(buffer, "exit") == 0){
        		run.store(false);
        	}else{
        	    ENetPacket* packet = enet_packet_create (buffer, 512, ENET_PACKET_FLAG_RELIABLE);
			    enet_host_broadcast (host.load(), 0, packet);         
			    enet_host_flush (host.load());	
			    printf("\033[1A"); //go up one line
			    printf("\033[K"); //delete to the end of the line
			    printf("\rServer: %s\n", buffer+1); // use \r to get back to the start and print
        	}
        }

    }
}