#include <musync/worker_thread.hpp>
namespace musync {
namespace server {
void connection::user_list_files() {
	std::vector< std::pair<std::string,std::string> > files;
	if(user_id_is_set) {
		if(data->list_files(user_id,files)) {
			std::stringstream ss;

			for(auto file_pair : files) {
				if(file_pair.first != username)
					ss << "'" << file_pair.first << "/"
						<< file_pair.second << "'\n";
				else
					ss << "'" << file_pair.second << "'\n";
			}
			std::size_t msg_size = ss.str().length();

			if(msg_size > 0) {
				// To ensure that what we send is valid we assign it
				// to a local variable
				list_string = std::to_string(msg_size - 1) + " " + ss.str();

				auto self = shared_from_this();
				asio::async_write(conn_socket,
						asio::buffer(list_string),
					[this,self](std::error_code /*ec*/, std::size_t /*bytes_sent*/) {
						finish();
					});
			} else {
				auto self = shared_from_this();
				asio::async_write(conn_socket, asio::buffer(reply::empty_library),
					[this,self](std::error_code /*ec*/, std::size_t /*bytes_sent*/) {
						finish();
					});
			}
		} else {
			std::cout << "data->list_files() failed" << std::endl;
		}
	} else {
		auto self = shared_from_this();
		asio::async_write(conn_socket, asio::buffer(reply::not_logged_in),
			[this,self](std::error_code /*ec*/, std::size_t /*bytes_sent*/) {
				finish();
			});
	}
}
} //namespace server
} //namespace musync
