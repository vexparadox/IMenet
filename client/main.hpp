#pragma once
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <thread>

//type def functions 
typedef void (* Action)(ENetEvent* event);

std::atomic<ENetHost*> client;
std::atomic<ENetPeer*> peer;
std::atomic<ENetPeer*> server;
ENetAddress clientAddress;
bool connected = false; // if we're connected or not
std::atomic<bool> running; // the running boolean
char** usernames; //storing all the usernames

void disconnect();
void takeInput();
void getUsername();
void messageRecieved(ENetEvent* event);
void newUser(ENetEvent* event);
void userDisconnected(ENetEvent* event);

//a list of actions
Action actions[] = {
	messageRecieved,
	newUser,
	userDisconnected
};