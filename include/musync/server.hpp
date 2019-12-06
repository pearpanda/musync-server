#ifndef MUSYNC_SERVER
#define MUSYNC_SERVER
#include <asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <filesystem>
#include <string>
#include <stdexcept>
#include <musync/config_vars.hpp>
#include <musync/connection_manager.hpp>
namespace musync {
namespace server {
	using tcp = asio::ip::tcp;
class server {
public:
	server(const server&) = delete;
	server& operator=(const server&) = delete;
	server();
	server(const config_vars& vars);
	server(const config_vars& vars, const std::string& config_file);

	void init();
	void run();
	void stop();
private:
	void do_accept();
	void handle_accept();
	config_ptr config;
	asio::io_context main_io_context;
	asio::signal_set signals;
	tcp::acceptor acceptor;
	tcp::socket acceptor_socket;
	connection_manager manager;
	connection_ptr current_connection;
};
}
}
#endif
