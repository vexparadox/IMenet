#include "server.hpp"
#include "message.hpp"
#include <utility>

namespace IMenet
{
    Server::Server(std::string_view host_name, uint32_t port)
        : m_broadcast_message(512)
    {
        m_initialised = enet_initialize() == 0;
        if (m_initialised)
        {
            enet_address_set_host(&m_server_address, host_name.data());
            m_server_address.port = port;
        }
    }

    Server::~Server()
    {
        stop();
        enet_deinitialize();
    }

    bool Server::run()
    {
        if (m_initialised == false)
        {
            return false;
        }

        m_host = enet_host_create(&m_server_address, 32, 2, 0, 0);
        if (m_host == nullptr)
        {
            return false;
        }

        m_running = true;

        m_thread = std::thread([&]()
                               {
            while(m_running.load())
            {
                ENetEvent event;
                constexpr int32_t poll_timeout_ms = 150;
                while (enet_host_service (m_host, &event, poll_timeout_ms) > 0)
                {
                    switch (event.type)
                    {
                    case ENET_EVENT_TYPE_CONNECT:
                    {
                            m_broadcast_message.reset();
                            printf ("A new client connected from %x:%u.\n", 
                                    event.peer -> address.host,
                                    event.peer -> address.port);
                            //make space for the new users ID
                            event.peer->data = new char(m_client_count);
                            //write the new client Count
                            printf("New connection with ID of: %x\n", m_client_count); // print the id of the new client
                            m_client_count++; // increment the id
                            m_broadcast_message.write(uint8_t(ServerActionType::MESSAGE_STRING)); // this is a message #TODO add enum for client actions
                            m_broadcast_message.write(uint8_t(255)); // say it's from the server
                            m_broadcast_message.write("Welcome to the Server.");
                            //create a packet and send
                            ENetPacket* welcome_packet = enet_packet_create(m_broadcast_message.buffer(), m_broadcast_message.size(), ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(event.peer, 0, welcome_packet);
                            enet_host_flush(m_host);

                            //tell the new user about existing users
                            // #TODO user id isn't capped here at 255
                            for(auto&[user_id, user] : m_users)
                            {
                                m_broadcast_message.reset();
                                m_broadcast_message.write(uint8_t(ServerActionType::USERNAME_REGISTRATION));
                                m_broadcast_message.write(user.user_id());
                                m_broadcast_message.write(user.username());
                                ENetPacket* username_packet = enet_packet_create(m_broadcast_message.buffer(), m_broadcast_message.size(), ENET_PACKET_FLAG_RELIABLE);
                                enet_peer_send(event.peer, 0, username_packet);
                                enet_host_flush(m_host);
                            }
                            break;
                        }
                    case ENET_EVENT_TYPE_RECEIVE:
                    {
                        if(event.packet != nullptr && event.packet->dataLength == 0)
                        {
                            fprintf (stderr, "Empty packet data recieved.\n");
                            break;
                        }

                        m_broadcast_message.reset();
                        MessageBufferRead event_data { event.packet->data, (uint32_t)event.packet->dataLength};

                        ClientActionType action_type = (ClientActionType)event_data.read_byte();
                        event_data.advance_byte(); // All client > server skips the second byte
                        uint8_t user_id = *static_cast<uint8_t*>(event.peer->data);
                        const User* user = get_user(user_id);
                        switch(action_type)
                        {
                            case ClientActionType::USERNAME_REGISTRATION:
                            {
                                if(event_data.size() <= 3)
                                {
                                    fprintf (stderr, "NEW_USER message recieved but no username was supplied.\n");
                                    break;
                                }
                                //save the new username given
                                if(register_user(event_data.read_string(), user_id) == false)
                                {
                                    // #TODO give error
                                    break;
                                }
                                printf("New user with name: %s\n", get_username(user_id).data());
                                // Broadcast the new user
                                m_broadcast_message.write(uint8_t(ServerActionType::USERNAME_REGISTRATION)); // This is a new user notification #TODO add enum for client actions
                                m_broadcast_message.write(uint8_t(user_id)); 
                                m_broadcast_message.write(get_username(user_id));
                                break;
                            }
                            case ClientActionType::MESSAGE_STRING:
                            {
                                if(user == nullptr)
                                {
                                    // Means we have an unregisterd user...
                                    fprintf(stderr, "Unregistered user on message print! %x\n", user_id);
                                    break;
                                }

                                m_broadcast_message.write(uint8_t());
                                m_broadcast_message.write(user_id);
                                std::string client_message = event_data.read_string();
                                m_broadcast_message.write(client_message);

                                // print on the server CLI
                                printf("%s: %s \n",
                                    user->username().data(), // the users username
                                    client_message.data()    // the users message
                                );

                                std::lock_guard lock(m_recieved_messages_mutex);
                                m_messages.emplace_back(*user, std::move(client_message));
                                break;
                            }
                        }
                        if(m_broadcast_message.size() > 0)
                        {
                            ENetPacket* packet = enet_packet_create(m_broadcast_message.buffer(), m_broadcast_message.size(), ENET_PACKET_FLAG_RELIABLE);
                            enet_host_broadcast(m_host, 0, packet);
                            enet_host_flush(m_host);
                        }
                        enet_packet_destroy(event.packet);
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT:
                    {
                        const uint8_t user_id = *static_cast<uint8_t*>(event.peer->data);
                        printf("%s disconnected.\n", get_username(user_id).data());
                        disconnect_user(user_id);
                        m_broadcast_message.reset();
                        m_broadcast_message.write(uint8_t(ServerActionType::USER_DISCONNECTED));
                        m_broadcast_message.write(user_id);
                        ENetPacket* packet = enet_packet_create(m_broadcast_message.buffer(), m_broadcast_message.size(), ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast (m_host, 0, packet);
                        enet_host_flush (m_host);
                        event.peer->data = nullptr;
                        break;
                    }
                    case ENET_EVENT_TYPE_NONE:
                        break;
                    }
                }
                send_pending_messages();
            }
            enet_host_destroy(m_host); });
        return true;
    }

    void Server::send_message(std::string &&message)
    {
        std::lock_guard lock(m_pending_messages_mutext);
        m_pending_messages.emplace_back(std::move(message));
    }

    void Server::send_pending_messages()
    {
        std::lock_guard lock(m_pending_messages_mutext);
        for (const std::string &message : m_pending_messages)
        {
            m_broadcast_message.reset();
            m_broadcast_message.write(uint8_t(ServerActionType::MESSAGE_STRING));
            m_broadcast_message.write(uint8_t(255));
            m_broadcast_message.write(message);
            ENetPacket *packet = enet_packet_create(m_broadcast_message.buffer(), m_broadcast_message.size(), ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(m_host, 0, packet);
            enet_host_flush(m_host);
        }
        m_pending_messages.clear();
    }

    bool Server::is_running()
    {
        return m_running.load();
    }

    void Server::stop()
    {
        m_running = false;
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    void Server::disconnect_user(uint8_t user_id)
    {
        m_users.erase(user_id);
    }

    const User* Server::get_user(uint8_t user_id) const
    {
        auto found = m_users.find(user_id);
        if(found != m_users.end())
        {
            return &found->second;
        }
        return nullptr;
    }

    const std::string &Server::get_username(uint8_t user_id) const
    {
        static std::string s_empty{};
        if(const User* user = get_user(user_id); user != nullptr)
        {
            return user->username();
        }
        return s_empty;
    }

    bool Server::register_user(std::string &&username, uint8_t user_id)
    {
        if(const User* user = get_user(user_id); user == nullptr)
        {
            m_users.emplace(std::make_pair(user_id, User{user_id, std::move(username)}));
            return true;
        }
        return false;
    }
}