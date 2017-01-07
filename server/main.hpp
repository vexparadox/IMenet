#pragma once
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <enet/enet.h>
#include <atomic>
#include <thread>

//type def functions 
typedef void (* Action)(ENetEvent* event);

ENetAddress serverAddress;
std::atomic<ENetHost*> host; // the Enet Host
std::atomic<bool> run; //the running bool
char clientCount; // the current count of users
char* broadcastMessage; // the char* that's used to broadcast messages
char** usernames; // a set of bools if the usernames have 
void takeInput();
void messageRecieved(ENetEvent* event);
void newUser(ENetEvent* event);
void userDisconnected(ENetEvent* event);
void sendBroadcast();

//a list of actions
Action actions[] = {
	messageRecieved,
	newUser
};