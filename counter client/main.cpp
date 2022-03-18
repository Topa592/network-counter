#include <iostream>
#include <asio.hpp>
#include <string>

using asio::ip::tcp;

int main(int argc, char* argv[]) {
	try {
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <string message>" << std::endl;
			return 1;
		}
		asio::io_context io_context;
		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "14517");
		tcp::socket socket(io_context);
		asio::connect(socket, endpoints);
		for (;;)
		{
			std::string message = argv[2];
			if (message.size() > 128) break;

			asio::error_code ignored_error;
			size_t bytesWritten = asio::write(socket, asio::buffer(message), ignored_error);
			if (bytesWritten > 0) break;
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}