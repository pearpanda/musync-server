#ifndef MUSYNC_CONNECTION_HPP
#define MUSYNC_CONNECTION_HPP
#include <memory> // std::shared_ptr
#include <iostream> // Debugging mostly
#include <string>
#include <fstream> // File I/O
#include <vector> // The default container
#include <sstream> // stringstream for list files
#include <exception>
#include <stdexcept>
#include <filesystem> //std::filesystem
#include <asio.hpp>
#include <musync/config_vars.hpp>
#include <musync/request.hpp>
#include <musync/db_link.hpp>
#include <musync/reply.hpp>
namespace musync {
namespace server {
class worker_thread;
class connection : public std::enable_shared_from_this<connection> {
public:
	connection(const connection&) = delete;
	connection& operator=(const connection&) = delete;

	explicit connection(std::shared_ptr<worker_thread> worker,
			std::shared_ptr<asio::io_context> io,
			config_ptr vars_global,
			db_link_ptr data_parent);
	~connection();

	asio::ip::tcp::socket& get_socket();
	void start();
	void reset();
	void finish();
private:
	void start_request();
	void get_request_size();
	void get_request();
	void execute_request();

	std::string reply_string;
	void send_reply();

	config_ptr vars;
	db_link_ptr data;
	std::shared_ptr<asio::io_context> io_context;
	std::shared_ptr<worker_thread> parent;
	asio::ip::tcp::socket conn_socket;
	char *buffer;
	std::size_t buffer_size;

	// Once the request executes fully,
	// a new request can be put in place
	request received_request;
	std::size_t request_size; // The total request size
	std::size_t current_size; // The size that is currently present
	std::size_t arg_size; // Number of arguments in a request
	std::size_t arg_index; // The index of the argument that will be processed

	// Individual actions, depending on request

	// User management
	bool user_id_is_set;
	std::string user_id; // The user_id of the user in string format
	std::string username;
	void user_login();
	void user_register();

	// List files in library
	std::string list_string;
	std::size_t list_string_size;
	void user_list_files();

	// File management
	std::fstream file;
	std::string filename;
	std::filesystem::path file_path;

	// Sending files
	void send_files();
	void send_file();

	// Receiveing files
	std::size_t num_files; // Number of files to receive
	std::size_t current_file; // The current file which is to be processed
	std::size_t received_file_size; // The size of the file to be received
	std::size_t current_file_size; // How much do we currently have?
	void start_add_files();
	void add_files();

	bool header_size_known;
	void get_file_header(std::size_t i, std::size_t bytes_read);
	void get_file_size();
	void add_file();

	// Removing files
	void remove_files();

	// File sharing
	void share_files();
	void revoke_share();
};
typedef std::shared_ptr<connection> connection_ptr;
}
}
#endif
