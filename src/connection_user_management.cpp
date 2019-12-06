#include <musync/worker_thread.hpp>
//TODO: Disable handling of slashes in request.cpp
namespace musync {
namespace server {
void connection::user_login() {
	auto self = shared_from_this();
	const std::vector<std::string>& args = received_request.get_arguments();
	if(arg_size == 2 && data->try_login(args[0],args[1],user_id)) {
		user_id_is_set = true;
		// After the user is logged in, the data present are the user_id and username
		data->username_from_id(user_id, username);

		reply_string = std::to_string(reply::login_success.length()) + " " + reply::login_success;
		// This is not a regular send reply, so send_reply() can't be used here
		asio::async_write(conn_socket, asio::buffer(reply_string),
				[this,self](std::error_code ec, std::size_t /*ignored*/) {
					if(!ec) {
						start_request();
					} else {
						finish();
					}
				});
	} else {
		reply_string = std::to_string(reply::login_failure.length()) + " " + reply::login_failure;
		send_reply();
	}
}
void connection::user_register() {
	const std::vector<std::string>& args = received_request.get_arguments();
	if(arg_size >= 4) {
		// args[0] = username
		// args[1] = email
		// args[2] = password
		// args[3] = first_name
		// args[4] = last_name
		if(args[0].find('/') != std::string::npos ||
				args[1].find('/') != std::string::npos ||
				args[0].find('@') != std::string::npos) {
			reply_string = std::to_string(reply::registration_failure.length()) + " "
				+ reply::registration_failure;
			send_reply();
			finish();
			return;
		}
		// Email must have an @ and a dot somewhere after the @ symbol
		// This isn't perfect and can probably be replaced with regex checking
		std::size_t email_at = args[1].find('@');
		if(email_at == std::string::npos || args[1].find('.',email_at) == std::string::npos) {
			reply_string = std::to_string(reply::registration_failure.length()) + " "
				+ reply::registration_failure;
			send_reply();
			finish();
			return;
		}
		// The username must not have only numbers
		bool username_is_all_numbers = true;
		for(auto c : args[0]) {
			if(!(c >= '0' && c <= '9')) {
				username_is_all_numbers = false;
				break;
			}
		}
		if(username_is_all_numbers) {
			reply_string = std::to_string(reply::registration_failure.length()) + " "
				+ reply::registration_failure;
			send_reply();
			finish();
			return;
		}

		// The results are valid so far
		if(arg_size == 4) {
			if(data->try_register(args[0],args[1],args[2],args[3])) {
				// Good results
				std::filesystem::create_directory(vars->files_root / args[0]);
				reply_string = std::to_string(reply::registration_success.length()) + " "
					+ reply::registration_success;
			} else {
				// Bad results
				reply_string = std::to_string(reply::registration_success.length()) + " "
					+ reply::registration_failure;
			}
		} else if(arg_size == 5) {
			if(data->try_register(args[0],args[1],args[2],args[3],args[4])) {
				// Good results
				std::filesystem::create_directory(vars->files_root / args[0]);
				reply_string = std::to_string(reply::registration_success.length()) + " "
					+ reply::registration_success;
			} else {
				// Bad results
				reply_string = std::to_string(reply::registration_success.length()) + " "
					+ reply::registration_failure;
			}
		} else {
			reply_string = std::to_string(reply::incorrect_number_of_arguments.length()) + " "
				+ reply::incorrect_number_of_arguments;
		}
		send_reply();
		finish();
	}
}
} //namespace server
} //namespace musync
