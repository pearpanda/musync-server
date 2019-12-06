#include <musync/worker_thread.hpp>
namespace musync {
namespace server {
void connection::send_files() {
	if(!user_id_is_set) {
		reply_string = std::to_string(reply::not_logged_in.length()) + " " + reply::not_logged_in;
		send_reply();
		finish();
		return;
	}
	const std::vector<std::string>& files = received_request.get_arguments();

	if(arg_index >= arg_size) {
		finish();
		return;
	}

	bool is_valid_filename;
	do {
		is_valid_filename = true; // True unless proved false
		filename = files[arg_index++];

		// Handle whether removal is in progress
		// through locks
		// If there is a removal, we cannot be sure that
		// the removal is affecting the file which we want to get
		std::size_t slash = filename.find('/');
		if(slash == std::string::npos) {
			is_valid_filename = data->is_file_in_library(user_id,username,filename);
			if(is_valid_filename) file_path = (vars->files_root / username) / filename;
		} else if(filename.find('/',slash + 1) == std::string::npos) {
			std::string song_username = filename.substr(0,slash);
			filename = filename.substr(slash + 1);
			is_valid_filename = data->is_file_in_library(user_id,song_username,filename);
			if(is_valid_filename) file_path = (vars->files_root / song_username) / filename;
		} else is_valid_filename = false;

		if(is_valid_filename) {
			is_valid_filename = std::filesystem::exists(file_path);
		}
	} while(arg_index < arg_size && !is_valid_filename);
	// All filenames have been exhausted
	if(!is_valid_filename) {
		finish();
		return;
	}

	file.open(file_path, std::fstream::in | std::fstream::binary);

	std::size_t file_size_bytes = std::filesystem::file_size(file_path);
	std::cout << filename << " has " << file_size_bytes << " bytes" << std::endl;

	reply_string = std::to_string(filename.length()) + " " + filename +
		std::to_string(file_size_bytes) + " ";

	auto self = shared_from_this();
	asio::async_write(conn_socket, asio::buffer(reply_string,reply_string.length()),
		[this,self] (std::error_code ec, std::size_t bytes_sent) {
			if(!ec) {
				send_file();
			} else {
				std::cerr << "ERROR: async_write:send_files()" << std::endl;
				std::cerr << "Cont. " << ec.message() << std::endl;
				finish();
			}
		}
	);
}

void connection::send_file() {
	std::size_t bytes_read;
	file.read(buffer,buffer_size);
	bytes_read = file.gcount();
	if(bytes_read) {
		auto self = shared_from_this();
		asio::async_write(conn_socket, asio::buffer(buffer,bytes_read),
			[this,self] (std::error_code ec, std::size_t bytes_sent) {
				if(!ec) {
					send_file();
				} else {
					std::cerr << "ERROR: async_write:send_file()" << std::endl;
					std::cerr << "Cont. " << ec.message() << std::endl;
					finish();
				}
			}
		);
	} else {
		file.close();
		send_files();
	}
}

void connection::start_add_files() {
	if(!user_id_is_set) {
		reply_string = std::to_string(reply::not_logged_in.length()) + " " + reply::not_logged_in;
		send_reply();
		finish();
		return;
	}
	const std::vector<std::string>& args = received_request.get_arguments();
	if(args.size() == 0) {
		reply_string = std::to_string(reply::incorrect_number_of_arguments.length()) + " " +
			reply::incorrect_number_of_arguments;
		send_reply();
		finish();
		return;
	}

	std::stringstream ss(args[0]);
	ss >> num_files;
	current_file = 0;

	reply_string = "OK: Start sending files";
	reply_string = std::to_string(reply_string.length()) + " " + reply_string;
	send_reply();
	add_files();
}

void connection::add_files() {
	if(current_file >= num_files) {
		finish();
		return;
	}

	received_file_size = 0;
	current_file_size = 0;

	filename.clear();

	header_size_known = false;
	current_size = 0;
	request_size = 0;

	auto self = shared_from_this();
	conn_socket.async_read_some(asio::buffer(buffer, buffer_size),
	[this,self](std::error_code ec, std::size_t bytes_read) {
		if(ec || bytes_read == 0) {
			std::cerr << "Early exit in get_file_header()" << std::endl;
			std::cerr << "Cont. " << ec.message() << std::endl;
			finish();
		} else {
			get_file_header(0,bytes_read);
		}
	});
}

void connection::get_file_header(std::size_t i, std::size_t bytes_read) {
	if(header_size_known) {
		std::size_t write_bytes = bytes_read - i;
		if(request_size < current_size + write_bytes)
			write_bytes = request_size - current_size;
		current_size += write_bytes;
		filename.append(buffer + i, write_bytes);
		if(current_size < request_size) {
			std::cout << current_size << " " << request_size << std::endl;
			// The size is known but the header is not yet fully received
			// Calling this function again to continue reading
			auto self = shared_from_this();
			conn_socket.async_read_some(asio::buffer(buffer, buffer_size),
			[this,self](std::error_code ec, std::size_t bytes_read) {
				if(ec || bytes_read == 0) {
					std::cerr << "Early exit in get_file_header()" << std::endl;
					std::cerr << "Cont. " << ec.message() << std::endl;
					finish();
				} else {
					get_file_header(0,bytes_read);
				}
			});
		} else {
			// Filename processing
			if(filename.find('/') != std::string::npos ||
					filename == "." || filename == ".." ||
					data->does_file_exist(user_id,filename) /*if it already exists*/) {
				reply_string = "ERROR: The filename '" + filename + "' is not valid.";
				reply_string = std::to_string(reply_string.length()) + " " + reply_string;
				send_reply();
				add_files();
			} else {
				reply_string = "OK: Start sending the file";
				reply_string = std::to_string(reply_string.length()) + " " + reply_string;
				file_path = (vars->files_root / username) / filename;
				file.open(file_path,std::fstream::out | std::fstream::binary);
				send_reply();
				get_file_size();
			}
		}
	} else {
		while(i < bytes_read && buffer[i] >= '0' && buffer[i] <= '9') {
			request_size *= 10;
			request_size += (buffer[i] - '0');
			++i;
		}
		if(i < bytes_read && buffer[i] == ' ') {
			header_size_known = true;
			get_file_header(i+1,bytes_read);
		} else {
			// Calling this function again to continue reading
			auto self = shared_from_this();
			conn_socket.async_read_some(asio::buffer(buffer, buffer_size),
			[this,self](std::error_code ec, std::size_t bytes_read) {
				if(ec || bytes_read == 0) {
					std::cerr << "Early exit in renewal of get_file_header()" << std::endl;
					std::cerr << "Cont. " << ec.message() << std::endl;
					finish();
				} else {
					get_file_header(0,bytes_read);
				}
			});
		}
	}
}

void connection::get_file_size() {
	auto self = shared_from_this();
	conn_socket.async_read_some(asio::buffer(buffer, buffer_size),
	[this,self](std::error_code ec, std::size_t bytes_read) {
		if(ec || bytes_read == 0) {
			std::cerr << "Early exit in get_file_size:async_read_some" << std::endl;
			finish();
		} else {
			std::size_t i = 0;
			while(i < bytes_read && buffer[i] >= '0' && buffer[i] <= '9') {
				received_file_size *= 10;
				received_file_size += (buffer[i] - '0');
				++i;
			}
			if(i < bytes_read && buffer[i] == ' ') {
				file.write(buffer + i + 1, bytes_read - i - 1);
				current_file_size = bytes_read - i - 1;
				if(current_file_size < received_file_size) {
					add_file();
				} else {
					file.close();
					++current_file;
					data->add_file(user_id,filename);
					add_files();
				}
			} else {
				get_file_size();
			}
		}
	});
}

void connection::add_file() {
	auto self = shared_from_this();
	conn_socket.async_read_some(asio::buffer(buffer,buffer_size),
		[this,self] (std::error_code ec, std::size_t bytes_read) {
			if(!ec) {
				file.write(buffer,bytes_read);
				current_file_size += bytes_read;
				if(current_file_size == received_file_size) {
					file.close();
					++current_file;
					data->add_file(user_id,filename);
					reply_string = "SUCCESS: Added file " + filename;
					reply_string = std::to_string(reply_string.length()) + " " + reply_string;
					send_reply();
					add_files();
				} else {
					add_file();
				}
			} else {
				std::cout << "Error in add_file\n\tFile size: " << received_file_size
					<< "\n\tCurrent file size: " << current_file_size;
				std::cout << ec.message() << std::endl;
				finish();
			}
		}
	);
}

void connection::remove_files() {
	if(!user_id_is_set) {
		reply_string = std::to_string(reply::not_logged_in.length()) + " " + reply::not_logged_in;
		send_reply();
		finish();
		return;
	}
	if(arg_size == 0) {
		reply_string = std::to_string(reply::incorrect_number_of_arguments.length()) + " " +
			reply::incorrect_number_of_arguments;
		send_reply();
		finish();
		return;
	}
	const std::vector<std::string>& args = received_request.get_arguments();
	while(arg_index < arg_size) {
		filename = args[arg_index++];
		std::size_t slash = filename.find('/');
		if(slash == std::string::npos) {
			bool is_valid_filename = data->remove_file(user_id,filename);
			if(is_valid_filename) {
				file_path = (vars->files_root / username) / filename;
				if(std::filesystem::is_regular_file(file_path)) std::filesystem::remove(file_path);
			}
		} else if(filename.find('/',slash + 1) == std::string::npos) {
			std::string song_username = filename.substr(0,slash);
			filename = filename.substr(slash + 1);
			data->remove_file_from_library(user_id,song_username,filename);
		}
	}
}

void connection::share_files() {
	if(!user_id_is_set) {
		reply_string = std::to_string(reply::not_logged_in.length()) + " " + reply::not_logged_in;
		send_reply();
		finish();
		return;
	}
	if(arg_size < 2) {
		reply_string = std::to_string(reply::incorrect_number_of_arguments.length()) + " " +
			reply::incorrect_number_of_arguments;
		send_reply();
		finish();
		return;
	}
	const std::vector<std::string>& args = received_request.get_arguments();
	std::string song_username = args[0];
	++arg_index;
	while(arg_index < arg_size) {
		filename = args[arg_index++];
		std::size_t slash = filename.find('/');
		if(slash == std::string::npos) {
			data->share(user_id,song_username,filename);
		}
	}
}

void connection::revoke_share() {
	if(!user_id_is_set) {
		reply_string = std::to_string(reply::not_logged_in.length()) + " " + reply::not_logged_in;
		send_reply();
		finish();
		return;
	}
	if(arg_size < 2) {
		reply_string = std::to_string(reply::incorrect_number_of_arguments.length()) + " " +
			reply::incorrect_number_of_arguments;
		send_reply();
		finish();
		return;
	}
	const std::vector<std::string>& args = received_request.get_arguments();
	std::string song_username = args[0];
	++arg_index;
	while(arg_index < arg_size) {
		filename = args[arg_index++];
		std::size_t slash = filename.find('/');
		if(slash == std::string::npos) {
			data->revoke_share(user_id,song_username,filename);
		}
	}
}
} //namespace server
} //namespace musync
