#pragma once
#include "shared.hpp"
#include <iostream>
#include <unordered_map>
#include <mutex>

namespace IMenet
{
	struct Message;
	class User;
	class Server final
	{
	public:
		Server(std::string_view host_name, uint32_t port);
		~Server();

		// starts a new thread that runs continuously, returns true if successful
		bool run();
		bool is_running();
		void stop();
		void send_message(std::string&& message);
		void send_pending_messages();

		std::vector<Message> poll(); // returns any of the gathered messages since the last poll
	private:
		const User* get_user(uint8_t user_id) const;
		const std::string& get_username(uint8_t user_id) const;
		bool register_user(std::string&& username, uint8_t user_id);
		void disconnect_user(uint8_t user_id);

		ENetAddress m_server_address;
		uint8_t m_client_count{};
		ENetHost* m_host{};
		std::atomic<bool> m_running{};
		std::unordered_map<uint8_t, User> m_users;

		std::mutex m_pending_messages_mutext, m_recieved_messages_mutex;
		std::vector<std::string> m_pending_messages;
		std::vector<Message> m_messages;
		std::thread m_thread;
		bool m_initialised{};

		OwningMessageBuffer m_broadcast_message;
	};
}
