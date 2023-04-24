#include "main.hpp"
#include "../../src/server.hpp"

int main(int argc, char const *argv[])
{
    if(argc < 3){
    	fprintf(stderr, "No address or port was supplied\n");
    	return EXIT_FAILURE;
    }
    std::string_view server_address = argv[1];
    int port_number = atoi(argv[2]);

    IMenet::Server server(server_address, port_number);

    if(server.run())
    {
        IMenet::start_input_loop(server);
    }
    return 0;
}