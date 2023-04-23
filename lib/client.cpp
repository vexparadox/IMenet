#include "client.hpp"

namespace IMenet
{
    Client::Client(std::string_view host_name, uint32_t port)
        : m_server_message(512), m_usernames(255, std::string{})
    {
        m_initialised = enet_initialize() == 0;
        if (m_initialised)
        {
            enet_address_set_host(&m_server_address, host_name.data());
            m_server_address.port = port;
        }
    }

    Client::~Client()
    {
        stop();
        enet_deinitialize();
    }

    bool Client::run()
    {
        if (m_initialised == false)
        {
            return false;
        }

        m_client.store(enet_host_create(NULL, 1, 2, 57600 / 8, 57600 / 8));
        if (m_client.load() == nullptr)
        {
            return false;
        }

        m_peer.store(enet_host_connect(m_client.load(), &m_server_address, 2, 0));
        enet_host_flush(m_client.load());

        ENetEvent event;
        if (enet_host_service(m_client.load(), &event, 2000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
        {
            printf("Connection succeeded.\n");
            m_server.store(event.peer);
            get_username_from_cli(); // blocking.. needs to move #TODO
        }
        else
        {
            enet_peer_reset(m_peer.load());
            return false;
            printf("%s\n", "Failed to connect.");
        }

        m_usernames[255] = "Server"; // special case for the server's id

        m_running.store(true);

        m_thread = std::thread([&]()
                               {
                                   while (m_running.load())
                                   {
                                       ENetEvent event;
                                       // wait upto half a second for an event
                                       while (enet_host_service(m_client.load(), &event, 500) > 0)
                                       {
                                           switch (event.type)
                                           {
                                           case ENET_EVENT_TYPE_RECEIVE:
                                           {
                                               MessageBufferRead event_data{event.packet->data, (uint32_t)event.packet->dataLength};
                                               ServerActionType action_type = (ServerActionType)event_data.read_byte();
                                               switch (action_type)
                                               {
                                               case ServerActionType::MESSAGE_STRING:
                                               {
                                                   printf("%s: %s \n", m_usernames[event_data.read_byte()].data(), event_data.read_string().data());
                                                   break;
                                               }
                                               case ServerActionType::USER_DISCONNECTED:
                                               {
                                                   const uint8_t disconnecting_user_id = event_data.read_byte();
                                                   printf("%s has disconnected.\n", m_usernames[disconnecting_user_id].data());
                                                   m_usernames[disconnecting_user_id] = {};
                                                   break;
                                               }
                                               case ServerActionType::USERNAME_REGISTRATION:
                                               {
                                                   // save the username that was given
                                                   const uint8_t new_user_id = event_data.read_byte();
                                                   std::string new_username = event_data.read_string();
                                                   printf("New user with the name: %s\n", new_username.data());
                                                   m_usernames[new_user_id] = std::move(new_username);
                                                   break;
                                               }
                                               }
                                               enet_packet_destroy(event.packet);
                                               break;
                                           }
                                           case ENET_EVENT_TYPE_DISCONNECT:
                                           {
                                               printf("Unexpectedly disconnected from the server!.\n");
                                               m_server.store(nullptr);
                                               m_peer.store(nullptr);
                                               m_running.store(false);
                                               break;
                                           }
                                           case ENET_EVENT_TYPE_CONNECT:
                                           case ENET_EVENT_TYPE_NONE:
                                               break;
                                           }
                                       }
                                       send_pending_messages();
                                   }
                                   disconnect(); // disconnect as the last thing
                               });

        return true;
    }

    void Client::send_message(std::string &&message)
    {
        std::lock_guard lock(m_pending_messages_mutext);
        m_pending_messages.emplace_back(std::move(message));
    }

    void Client::send_pending_messages()
    {
        std::lock_guard lock(m_pending_messages_mutext);
        for (const std::string &message : m_pending_messages)
        {
            m_server_message.reset();
            m_server_message.write((uint8_t)ClientActionType::MESSAGE_STRING);
            m_server_message.write((uint8_t)0); // unused, the server works out who this user is based on their connection
            m_server_message.write(message);
            ENetPacket *packet = enet_packet_create(m_server_message.buffer(), m_server_message.size(), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(m_server.load(), 0, packet);
            enet_host_flush(m_client.load());
        }
        m_pending_messages.clear();
    }

    bool Client::is_running()
    {
        return m_running.load();
    }

    void Client::stop()
    {
        m_running.store(false);
        m_thread.join();
    }

    const std::string &Client::get_username(uint8_t user_id) const
    {
        static std::string s_empty{};
        if (user_id < m_usernames.size())
        {
            return m_usernames[user_id];
        }
        return s_empty;
    }

    void Client::disconnect()
    {
        ENetEvent event;
        enet_peer_disconnect(m_peer.load(), 0);
        /* Allow up to 3 seconds for the disconnect to succeed
         * and drop any packets received packets.
         */
        printf("Attempting disconnect... \n");
        while (enet_host_service(m_client.load(), &event, 3000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s\n", "Disconnection succeeded");
                return;
            default:
                break;
            }
            // reset if we reach here, this is a forcefull disconnect
            enet_peer_reset(m_peer.load());
        }
    }

    void start_input_loop(IMenet::Client &client)
    {
        char buffer[510];
        while (client.is_running())
        {
            memset(buffer, 0, 510);
            if (fgets(buffer, 510, stdin) == nullptr)
            {
                continue;
            }
            // get rid of that pesky \n
            char *temp = buffer + strlen(buffer) - 1;
            *temp = '\0';
            if (strcmp(buffer, "") != 0)
            {
                if (strcmp(buffer, "exit") == 0)
                {
                    client.stop();
                }
                else
                {
                    client.send_message({&buffer[0]});
                    printf("\033[1A"); // go up one line
                    printf("\033[K");  // delete to the end of the line
                }
            }
        }
    }

    void Client::get_username_from_cli()
    {
        OwningMessageBuffer message{512};
        char buffer[510];
        memset(buffer, 0, 510);
        do
        {
            printf("Username: ");
            fgets(buffer, 510, stdin);
        } while (strlen(buffer) <= 1);

        message.write((uint8_t)ClientActionType::USERNAME_REGISTRATION);
        message.write((uint8_t)0); // unused, the server works out who this user is based on their connection

        char *temp = buffer + strlen(buffer) - 1; // remove the \n and replace with a null terminator
        *temp = '\0';

        // write the username to the buffer
        message.write(std::string{&buffer[0]});

        // send the username
        ENetPacket *packet = enet_packet_create(message.buffer(), message.size(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(m_server.load(), 0, packet);
        enet_host_flush(m_client.load());
    }

}