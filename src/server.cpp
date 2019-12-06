#include <musync/server.hpp>
namespace musync {
namespace server {
server::server()
	:config(std::make_shared<config_vars>()),
	main_io_context(),
	signals(main_io_context),
	acceptor(main_io_context),
	acceptor_socket(main_io_context),
	manager(config) {
		init();
}

server::server(const config_vars& vars)
	:config(std::make_shared<config_vars>(vars)),
	main_io_context(),
	signals(main_io_context),
	acceptor(main_io_context),
	acceptor_socket(main_io_context),
	manager(config) {
		init();
}

server::server(const config_vars& vars, const std::string& config_file)
	:config(std::make_shared<config_vars>(vars,config_file)),
	main_io_context(),
	signals(main_io_context),
	acceptor(main_io_context),
	acceptor_socket(main_io_context),
	manager(config) {
		init();
}

void server::init() {
	if(!std::filesystem::exists(config->files_root))
		throw std::runtime_error("The root folder ("+config->files_root.string()+") does not exist.");

	if(!std::filesystem::is_directory(config->files_root))
		throw std::runtime_error("The root folder specified "+config->files_root.string()+" is not a directory.");

	// Registrovanje signala
	signals.add(SIGINT);
	signals.add(SIGTERM);
	// SIGQUIT nije definisan na svim sistemima
#ifdef SIGQUIT
	signals.add(SIGQUIT);
#endif
	signals.async_wait(
		// Handling the incoming signals
		[this](std::error_code error,int signal_code) {
			stop();
		}
	);

	// Pravljenje acceptora koji ce slusati na adresi i portu, preko socketa acceptor_socket
	tcp::resolver resolver(main_io_context);
	tcp::resolver::query query(config->ip_host, config->port);
	tcp::endpoint endpoint = *resolver.resolve(query);
	acceptor.open(endpoint.protocol());
	acceptor.set_option(tcp::acceptor::reuse_address(true));
	acceptor.bind(endpoint);
	acceptor.listen();
	do_accept();
}

void server::run() {
	main_io_context.run();
}

void server::stop() {
	manager.stop();
	main_io_context.stop();
}

void server::do_accept() {
	current_connection = manager.get_available_connection();
	acceptor.async_accept(current_connection->get_socket(),
		[this](std::error_code error) {
			if(error) {
				std::cerr << "ERROR " << error.value() << ": " << error.message() << std::endl; //TODO: replace with logging when daemonising the server
				current_connection->finish();
			} else {
				current_connection->start();
				do_accept();
			}
		}
	);
}
}
}
