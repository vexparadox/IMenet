#pragma once
#include "user.hpp"
#include <string>

namespace IMenet
{
    struct Message
    {
        Message(const User& _user, std::string&& _message) : user(_user), message(std::move(_message))
        {
        }
        
        const User& user;
        std::string message;
    };
}