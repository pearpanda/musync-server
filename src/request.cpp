#include <musync/request.hpp>
namespace musync {
namespace server {
request::request()
	:request_msg(),
	type(request_type::bad_request),
	arguments() {}

void request::allocate(std::size_t msg_size) {
	request_msg.reserve(msg_size + 1);
}

void request::add(const char* buffer, const std::size_t& length) {
	request_msg.append(buffer,length);
}

void request::add(const std::string& append_str) {
	request_msg += append_str;
}

request::request_type request::get_request_type() {
	return type;
}

const std::vector<std::string>& request::get_arguments() {
	return arguments;
}

void request::parse_arguments(std::size_t begin, const std::string& whitespace) {
	begin = request_msg.find_first_not_of(whitespace,begin);
	request_size = request_msg.length();

	while(begin != std::string::npos) {
		std::string argument = parse_single_argument(begin);
		if(!argument.empty()) arguments.push_back(argument);
		begin = request_msg.find_first_not_of(whitespace, begin);
	}
}

std::string request::parse_quoted_argument(std::size_t& section_begin,
		const std::string& whitespace) {
	std::size_t section_end;
	std::string argument;

	if(request_msg[section_begin] == '\'') {
		// If the argument begins with a single quote, then it is closed by a single quote
		section_end = request_msg.find('\'', section_begin + 1);

		// No closing quote
		if(section_end == std::string::npos)
			return "";

		// The single quote is escaped by a backslash (\') so we need to find a quote that isn't escaped
		while(section_end != std::string::npos && request_msg[section_end-1] == '\\'
				&& !(section_end - section_begin > 2 && request_msg[section_end-2] == '\\'))
			section_end = request_msg.find('\'',section_end + 1);

		// No closing quote
		if(section_end == std::string::npos)
			return "";

		// At this point, the escaped quotes need to be reverted back to normal
		argument = request_msg.substr(
				section_begin + 1, section_end - section_begin - 1);
		section_begin = section_end + 1;
		unescape_string(argument);
		if(argument.compare(0,1,".") == 0 || argument.compare(0,2,"..") == 0)
			return "";
		else return argument;
	} else return "";
}

std::string request::parse_regular_argument(std::size_t& section_begin,
		const std::string& delimiters) {
	std::size_t section_end;
	std::string argument;
	section_end = request_msg.find_first_of(delimiters, section_begin + 1);
	while(section_end != std::string::npos &&
			request_msg[section_end-1] == '\\' &&
			!(section_end - section_begin > 2 &&
				request_msg[section_end-2] == '\\'))
		section_end = request_msg.find_first_of(delimiters, section_end + 1);
	if(section_end == std::string::npos) {
		argument = request_msg.substr(section_begin, section_end);
	} else {
		argument = request_msg.substr(section_begin, section_end - section_begin);
	}
	section_begin = section_end;
	unescape_string(argument);
	if(argument.compare(0,1,".") == 0 || argument.compare(0,2,"..") == 0)
		return "";
	return argument;
}

std::string request::parse_single_argument(std::size_t& begin,
		const std::string& whitespace) {
	std::string argument = "", part_arg;
	bool is_valid_argument = true;
	if(begin < request_size) {
		do {
			if(request_msg[begin] == '\'')
				part_arg = parse_quoted_argument(begin);
			else
				part_arg = parse_regular_argument(begin);
			if(part_arg.empty()) {
				is_valid_argument = false;
			}
			if(is_valid_argument)
				argument += part_arg;
		} while(begin != std::string::npos && begin < request_size &&
				whitespace.find(request_msg[begin]) == std::string::npos); // The character is a whitespace

		if(!is_valid_argument) return "";
		return argument;
	} else {
		return "";
	}
}

void request::unescape_string(std::string& src) {
	std::size_t str_length = src.length();
	std::size_t right_shift = 0;
	if(str_length > 1 && src[0] == '\\') {
		right_shift = 1;
	}
	for(std::size_t i = 1; i < str_length; ++i) {
		if(src[i] == '\\' && src[i-1] != '\\') {
			++right_shift;
		}
		else src[i-right_shift] = src[i];
	}
	src.resize(str_length - right_shift);
}

void request::process() {
	// Only syntax error checking is done here.
	// Semantic analysis is done later, when arguments are processed.

	request_size = request_msg.length();
	if(request_size == 0) return;
	std::string whitespace = " \t";
	if(whitespace.find(request_msg[0]) != std::string::npos)
		return;
	std::size_t begin = request_msg.find_first_of(whitespace);
	std::string command = request_msg.substr(0,begin);
	if(command == "ls") {
		type = request_type::list_files;
	} else if(command == "list") {
		type = request_type::list_files;
	} else if(command == "get") {
		type = request_type::send_files;
	} else if(command == "rm") {
		type = request_type::remove_files;
	} else if(command == "del") {
		type = request_type::remove_files;
	} else if((command == "remove") ||
			(command == "delete")) {
		type = request_type::remove_files;
	} else if(command == "add") {
		type = request_type::add_new_files;
	} else if(command == "new") {
		type = request_type::register_user;
	} else if(command == "register") {
		type = request_type::register_user;
	} else if(command == "login") {
		type = request_type::login;
	} else if(command == "update") {
		type = request_type::update_file_info;
	} else if(command == "upd") {
		type = request_type::update_file_info;
	} else if(command == "share") {
		type = request_type::share_file;
	} else if(command == "revoke") {
		type = request_type::revoke_share;
	} else {
		type = request_type::bad_request;
		return;
	}

	// It tries to parse the arguments
	// If the parsing fails, that means the query can't be executed,
	// and so the request is bad
	// Otherwise, the type will retain what it was assigned from the code
	// above, and the arguments will be added. Some statements may
	// ignore extra arguments
	parse_arguments(begin);
}
} //namespace server
} //namespace musync

// No more than one forward slash (/) is allowed
/*
std::size_t slash = argument.find('/');
if(slash == std::string::npos) return argument;
if(argument.find('/', slash + 1) == std::string::npos) { // There is one slash
	if(argument.substr(0, slash).empty() ||
			argument.substr(slash + 1, argument.length() - slash - 1).empty())
		return "";
	return argument;
}
return "";
*/
