#include <string>
#include <asio.hpp>
#include <memory>
namespace musync {
namespace server {
namespace reply {
	//TODO: add reply lengths to these replies
	const std::string not_logged_in = "ERROR: You are not logged in";
	const std::string login_success = "SUCCESS: Logged in.";
	const std::string login_failure = "ERROR: The username/email or password are incorrect.";
	const std::string registration_success = "SUCCESS: Registration successful.";
	const std::string registration_failure = "ERROR: Registration unsuccessful.";
	const std::string incorrect_number_of_arguments = "ERROR: You have specified an incorrect number of arguments.";
	const std::string empty_library = "NOTE: Your library is empty";
} //namespace reply
} //namespace server
} //namespace musync
