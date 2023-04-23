#pragma once
#include "shared.hpp"
#include <iostream>
#include <mutex>

namespace IMenet
{
	class Client final
	{
	public:
		Client(std::string_view host_name, uint32_t port);
		~Client();

		// starts a new thread that runs continuously, returns true if successful
		bool run();
		bool is_running();
		void stop();

		void send_message(std::string&& message);
		void send_pending_messages();
	private:
		const std::string& get_username(uint8_t user_id) const;
		void get_username_from_cli();
        void disconnect();

		ENetAddress m_server_address;
        ENetHost* m_client{};
        ENetPeer* m_peer{};
        ENetPeer* m_server{};

		std::atomic<bool> m_running{};
		std::vector<std::string> m_usernames;

		std::mutex m_pending_messages_mutext;
		std::vector<std::string> m_pending_messages;
		std::thread m_thread;
		bool m_initialised{};

		OwningMessageBuffer m_server_message;
	};

	void start_input_loop(IMenet::Client& client);
}
