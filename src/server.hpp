#pragma once
#include "shared.hpp"
#include <iostream>
#include <mutex>

namespace IMenet
{
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
	private:
		const std::string& get_username(uint8_t user_id) const;

		ENetAddress m_server_address;
		uint8_t m_client_count{};
		ENetHost* m_host{};
		std::atomic<bool> m_running{};
		std::vector<std::string> m_usernames;

		std::mutex m_pending_messages_mutext;
		std::vector<std::string> m_pending_messages;
		std::thread m_thread;
		bool m_initialised{};

		OwningMessageBuffer m_broadcast_message;
	};
}
