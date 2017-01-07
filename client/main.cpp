#include "main.hpp"
int main(int argc, char const *argv[])
{
	running.store(true);
    if (enet_initialize () != 0)
    {
        fprintf (stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    atexit (enet_deinitialize);
    if(argc < 3){
    	fprintf(stderr, "No address or port was supplied.\n");
    	return EXIT_FAILURE;
    }
    //create a client
    client.store(enet_host_create (NULL, 1, 2, 57600 / 8, 57600 / 8));
    //if the client failed to create
    if (client.load() == NULL){
	    fprintf (stderr, "An error occurred while trying to create an ENet client host.\n");
	    exit (EXIT_FAILURE);
	}
	//setup the clientAddress
	enet_address_set_host (&clientAddress, argv[1]);
    clientAddress.port = atoi(argv[2]);

    ENetEvent event;
    peer.store(enet_host_connect (client.load(), &clientAddress, 2, 0));
    enet_host_flush (client.load());
    if (!peer.load()){
    	printf("%s\n", "No available peers for initiating an ENet connection.");
    }

    printf("%s\n", "Attempting to connect...");

    if (enet_host_service (client.load(), &event, 2000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
    	printf("Connection to %s:%s succeeded.\n", argv[1], argv[2]);
        connected = true;
        server.store(event.peer);
        getUsername();
    }else{
        enet_peer_reset (peer.load());
        connected = false;
        printf("%s\n", "Failed to connect.");
        running.store(false);
    }

    //setup the usernames array
    usernames = (char**)malloc(sizeof(char)*255*sizeof(char*));
    for(int i = 0; i < 255; i++){
        usernames[i] = (char*)malloc(sizeof(char)*510);
        memset(usernames[i], 0, 510);
    }
    usernames[255] = "Server"; // set the special 255 to Server

    //start the input thread
    std::thread inputThread(&takeInput);
    //start the main loop
    while(running.load() && connected){
        ENetEvent event;
        //wait upto half a second for an event
        while (enet_host_service (client.load(), &event, 500) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:{
                actions[event.packet->data[0]](&event);
            }break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconnected.\n", usernames[*(char*)event.peer->data]);
                event.peer -> data = NULL;
                running.store(false);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }

    }
    inputThread.join();
    disconnect();
    return 0;
}

void getUsername(){
    char buffer[510];
    char message[512];
    memset(buffer, 0, 510);
    memset(message, 0, 512);
    do{
        printf("Username: ");
        fgets(buffer, 510, stdin);
    }while(strlen(buffer) <= 1);

    message[0] = 1; // set this to a 1, means a new user
    char* temp = buffer+strlen(buffer)-1; // remove the \n
    *temp = '\0';
    //copy the username to the buffer
    memcpy(message+2, buffer, 510);
    //send the username
    ENetPacket* packet = enet_packet_create (message, 512, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send (server.load(), 0, packet);         
    enet_host_flush (client.load());
}

void takeInput(){
    char message[512];
    char buffer[510];
    while (running.load()){
        //clear the buffer and message
        memset(buffer, 0, 510);
        memset(message, 0, 512);
        //get the buffer
        fgets(buffer, 510, stdin);
        //get rid of that pesky \n
        char* temp = buffer+strlen(buffer)-1;
        *temp = '\0';
        //set the params of the message
        message[0] = 0; // tell the server it's a message
        message[1] = 0; // the second byte is ignored by the server, it's filled with the userID when it's resent out
        if(strcmp(buffer, "") != 0){
            if(strcmp(buffer, "exit") == 0){
        		running.store(false);
        	}else{
                memcpy(message+2, buffer, 510); // copy the buffered string into the message
        	    ENetPacket* packet = enet_packet_create (message, 512, ENET_PACKET_FLAG_RELIABLE);
			    enet_peer_send (server.load(), 0, packet);         
			    enet_host_flush (client.load());
			    printf("\033[1A"); //go up one line
			    printf("\033[K"); //delete to the end of the line	
        	}
        }
    }
}

void userDisconnected(ENetEvent* event){
    printf("%s has disconnected.\n", usernames[event->packet->data[1]]);
    memset(usernames[event->packet->data[1]], 0, 255);
}

void messageRecieved(ENetEvent* event){
    //print the packet
    printf ("%s: %s \n",
            usernames[event->packet -> data[1]],
            event->packet -> data+2
            );
    enet_packet_destroy (event->packet);
}

void newUser(ENetEvent* event){
    //save the username that was given
    memcpy(usernames[event->packet->data[1]], event->packet->data+2, 510); 
    //print to tell the user
    printf("New user with the name: %s\n", usernames[event->packet->data[1]]);
    enet_packet_destroy (event->packet);
}

void disconnect(){
    ENetEvent event;
    enet_peer_disconnect (peer.load(), 0);
    /* Allow up to 3 seconds for the disconnect to succeed
    * and drop any packets received packets.
    */
    printf("Attempting disconnect... \n");
    while (enet_host_service (client.load(), & event, 3000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy (event.packet);
        break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("%s\n", "Disconnection succeeded");
        return;
        }
    }
    //reset if we reach here, this is a forcefull disconnect
    enet_peer_reset (peer.load());
}