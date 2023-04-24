#include "main.hpp"
#include "../../src/client.hpp"
#include "../../src/inputhelper.hpp"
#include <thread>

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        std::cerr << "No address or port was supplied." << std::endl;
        return EXIT_FAILURE;
    }
    std::string_view server_address = argv[1];
    int port_number = atoi(argv[2]);

    std::cout << "Attemping to connect..." << std::endl;
    IMenet::Client client(server_address, port_number);
    IMenet::InputHelper helper;

    if (client.run() == false || helper.run() == false)
    {
        std::cerr << "Failed to connect to the server" << std::endl;
        return -1;
    }

    while (client.is_running())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        std::vector<std::string> inputs = helper.poll();
        for (std::string &input : inputs)
        {
            if (input == "exit")
            {
                client.stop();
                helper.stop();
            }
            else
            {
                client.send_message(std::move(input));
            }
        }
    }
    return 0;
}