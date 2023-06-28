#pragma once
#include <string>
namespace IMenet
{
    class User
    {
    public:
        User(uint8_t user_id, std::string&& username) : m_user_id(user_id), m_username(std::move(username))
        {
        }
        uint8_t user_id() const { return m_user_id; }
        const std::string &username() const { return m_username; }

    private:
        uint8_t m_user_id{};
        std::string m_username;
    };
}