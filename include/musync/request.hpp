#ifndef MUSYNC_REQUEST_HPP
#define MUSYNC_REQUEST_HPP
#include <vector>
#include <string>
#include <iostream>
namespace musync {
namespace server {
class request {
public:
	request();
	enum class request_type {
		// Musync-Alpha protocol
		bad_request,
		// add <n>
		add_new_files,
		// ls (list)
		list_files,
		// get '<file_ID>'{'<file_ID>'}
		send_files,
		// rm (remove/delete/del) '<file_ID>'{'<file_ID>'}
		remove_files,
		// update (alter) '<file_ID>' set
			// tag <tag_name>
			// author <artist_name>
			// genre <genre>
			// album <album>
		update_file_info,
		// share <username_or_email> <filename>

		// Musync-Beta protocol
		// new (register) <username> <email> <password> <first_name> {<last_name>}
		register_user,
		// login <username> <password>
		login,
		// share <user_identifier> <filenames>
		share_file,
		revoke_share

		// Musync-Gamma protocol
		// Still undefined
	};
	void allocate(std::size_t msg_size);
	void add(const char *input, const std::size_t& length);
	void add(const std::string& append_str);
	void process();
	request_type get_request_type();
	const std::vector<std::string>& get_arguments();
private:
	std::string request_msg;
	std::size_t request_size;
	request_type type;
	std::vector<std::string> arguments;

	void parse_arguments(std::size_t begin,
			const std::string& whitespace = " \t");
	std::string parse_single_argument(std::size_t& section_begin,
			const std::string& whitespace = " \t");
	std::string parse_quoted_argument(std::size_t& section_begin,
			const std::string& whitespace = " \t");
	std::string parse_regular_argument(std::size_t& section_begin,
			const std::string& delimiters = " \t'");
	void unescape_string(std::string& src);
};
}
}
#endif
