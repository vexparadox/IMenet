#pragma once

#include <enet/enet.h>
#include <stdio.h>
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include "messagebuffer.hpp"

namespace IMenet
{
    // Client to server
    enum class ClientActionType : uint8_t
    {
        MESSAGE_STRING = 0,
        USERNAME_REGISTRATION = 1
    };

    // Server to client
    enum class ServerActionType : uint8_t
    {
        MESSAGE_STRING = 0,
        USERNAME_REGISTRATION = 1,
        USER_DISCONNECTED = 2,
    };

}