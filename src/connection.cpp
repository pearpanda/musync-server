#include <musync/worker_thread.hpp>
namespace musync {
namespace server {
connection::connection(std::shared_ptr<worker_thread> worker,
		std::shared_ptr<asio::io_context> io,
		config_ptr vars_global,
		db_link_ptr data_parent)
	:vars(vars_global),
	data(data_parent),
	io_context(io),
	parent(worker),
	conn_socket(*io_context),
	buffer_size(vars->buffer_size) {
		user_id_is_set = false;
		buffer = new char[vars->buffer_size];
}

connection::~connection() {
	delete[] buffer;
}

void connection::start() {
	start_request();
}

void connection::reset() {
	user_id_is_set = false;
	conn_socket = asio::ip::tcp::socket(*io_context);
}

void connection::finish() {
	std::error_code ec;
	conn_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
	conn_socket.close();
	parent->handle_finish(shared_from_this());
}

asio::ip::tcp::socket& connection::get_socket() {
	return conn_socket;
}

void connection::start_request() {
	received_request = request();
	request_size = 0;
	current_size = 0;
	arg_size = 0;
	arg_index = 0;

	// Start getting the request
	get_request_size();
}

void connection::get_request_size() {
	auto self = shared_from_this();
	conn_socket.async_read_some(asio::buffer(buffer, buffer_size),
	[this, self](std::error_code ec, std::size_t bytes_read) {
		if(ec || bytes_read == 0) {
			if(ec) std::cout << ec.message() << std::endl;
			finish();
		} else {
			std::size_t i = 0;
			while(i < bytes_read && buffer[i] >= '0' && buffer[i] <= '9') {
				request_size *= 10;
				request_size += (buffer[i] - '0');
				++i;
			}

			// If the request size is larger than the maximum (for example fake requests) then end the connection
			if(vars->max_request_size /* > 0 */ && request_size > vars->max_request_size) finish();
			if(i < bytes_read && buffer[i] == ' ') {
				try {
					// We try to allocate enough size for the entire request
					received_request.allocate(request_size);

					// The async_read_some function may read more than just the size of the
					// request, so what remains in the buffer after the space separator
					// is sent to the request
					current_size = bytes_read - i - 1;
					received_request.add(buffer + i + 1, current_size);

					// If the entire request hasn't yet been received, start receiving it
					// Otherwise just process the request, and if it is valid, execute it
					if(current_size < request_size)
						get_request();
					else {
						received_request.process();
						execute_request();
					}
				} catch(std::exception& e) {
					// Possible std::bad_alloc error
					std::cerr << e.what() << std::endl;
					finish();
				}
			} else {
				// The separator between the request size and the request itself hasn't been encountered,
				// so the search must continue
				// Under normal circumstances the buffer is large enough that this will never execute,
				// and the requests are of such size that they will not fill the buffer

				get_request_size();
			}
		}
	});
}

void connection::get_request() {
	auto self = shared_from_this();
	conn_socket.async_read_some(asio::buffer(buffer,buffer_size),
		[this, self](std::error_code ec, std::size_t bytes_read) {
			if(!ec) {
				if(current_size + bytes_read > request_size) {
					bytes_read = request_size - current_size;
				}
				current_size += bytes_read;
				received_request.add(buffer, bytes_read);
				if(current_size < request_size)
					get_request();
				else {
					received_request.process();
					execute_request();
				}
			} else {
				finish();
				std::cerr << ec.message() << std::endl;
			}
		});
}


void connection::execute_request() {
	using request_type = request::request_type;
	request_type t = received_request.get_request_type();
	arg_size = received_request.get_arguments().size();
	arg_index = 0;

	switch(t) {
		case request_type::login:
			user_login(); break;
		case request_type::register_user:
			user_register(); break;
		case request_type::add_new_files:
			start_add_files(); break;
		case request_type::list_files:
			user_list_files(); break;
		case request_type::send_files:
			send_files(); break;
		case request_type::remove_files:
			remove_files(); break;
		case request_type::share_file:
			share_files(); break;
		case request_type::revoke_share:
			revoke_share(); break;
//		case request_type::update_file_info:
//			update_file_info(); break;
		case request_type::bad_request:
			finish(); break;
	}
}

void connection::send_reply() {
	auto self = shared_from_this();
	asio::async_write(conn_socket,asio::buffer(reply_string,reply_string.length()),
			[this,self](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
			{}
		);
}
} //namespace server
} //namespace musync
