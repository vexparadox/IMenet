#pragma once
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <atomic>
#include <thread>
std::atomic<ENetHost*> client;
std::atomic<ENetPeer*> peer;
std::atomic<ENetPeer*> server;
ENetAddress clientAddress;
bool connected = false;
std::atomic<bool> running;


void disconnect();
void takeInput();