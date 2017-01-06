#pragma once
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <enet/enet.h>
#include <atomic>
#include <thread>

ENetAddress serverAddress;
std::atomic<ENetHost*> host;
std::atomic<bool> run;
char clientCount;
char* broadcastMessage;
int usernamesGiven[512];
void takeInput();