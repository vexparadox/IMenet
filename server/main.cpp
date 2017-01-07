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
    std::thread inputThread(&takeInput);

    //setup the usernames array
    usernames = (char**)malloc(sizeof(char)*255*sizeof(char*));
    for(int i = 0; i < 255; i++){
        usernames[i] = (char*)malloc(sizeof(char)*510);
        memset(usernames[i], 0, 510);
    }
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
                    //make space for the new users ID
					event.peer->data = malloc(sizeof(char));
                    //write the new client Count
					*(char*)event.peer->data = clientCount;
					printf("New connection with ID of: %x\n", clientCount); // print the id
					clientCount++; // increment the id
                    broadcastMessage[0] = 0; //this is a message
                    broadcastMessage[1] = 255; // say it's from the server
                    char tempMessage[] = "Welcome to the Server.";
                    //copy the welcome message
                    memcpy(broadcastMessage+2, tempMessage, strlen(tempMessage)+1);
                    //create a packet and send
	        	    ENetPacket* packet = enet_packet_create (broadcastMessage, 512, ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send (event.peer, 0, packet);
				    enet_host_flush (host.load());
                }break;
            case ENET_EVENT_TYPE_RECEIVE:{
                //call the action that corrosponds with the first byte
                //see the READEME
                actions[event.packet->data[0]](&event);
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
        char* temp = buffer+strlen(buffer+2)-1;
        *temp = '\0';
        buffer[0] = 0; // set the packet type to message
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

void newUser(ENetEvent* event){
    memset(broadcastMessage, 0, 512);
    broadcastMessage[0] = 1;
    broadcastMessage[1] = *(char*)event->peer->data;
    memcpy(usernames[*(char*)event->peer->data], event->packet->data+2, 510); // save the new username
    memcpy(broadcastMessage+2, event->packet->data+2, 510); // copy the new username for broadcast
    printf("New user with name: %s\n", usernames[*(char*)event->peer->data]);
    ENetPacket* packet = enet_packet_create (broadcastMessage, 512, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (host.load(), 0, packet);
    enet_host_flush (host.load());
    enet_packet_destroy (event->packet);
}

void messageRecieved(ENetEvent* event){
    memset(broadcastMessage, 0, 512);
    //if they have, just rebroadcast the message
    //send out the data to everyone else
    memcpy(broadcastMessage, event->packet->data, 512); // copy the message data
    broadcastMessage[1] = *(char*)event->peer->data; // stick the id in the second byte
    //print on the server
    printf ("%s: %s \n",
            usernames[broadcastMessage[1]], // the users id
            broadcastMessage+2 // the users message
            );
    ENetPacket* packet = enet_packet_create (broadcastMessage, 512, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast (host.load(), 0, packet);
    enet_host_flush (host.load());
    enet_packet_destroy (event->packet);
}