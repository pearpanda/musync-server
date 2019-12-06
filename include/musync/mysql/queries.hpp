#include <string>
namespace musync {
namespace server {
namespace query {
std::string login(const std::string& user_or_email,
		const std::string& passwd);

std::string user_register(
		const std::string& username,
		const std::string& email,
		const std::string& passwd,
		const std::string& first_name);

std::string user_register(
		const std::string& username,
		const std::string& email,
		const std::string& passwd,
		const std::string& first_name,
		const std::string& last_name);

std::string next_file_id(
		const std::string& user_id);

std::string list_files(
		const std::string& user_id);

std::string is_file_in_library(
		const std::string& library_owner_id,
		const std::string& song_owner_name,
		const std::string& song_filename);

std::string user_id_to_username(
		const std::string& user_id);

std::string file_exists(
		const std::string& user_id,
		const std::string& filename);

std::string file_id_to_filename(
		const std::string& user_id,
		const std::string& song_id);

std::string add_file(
		const std::string& user_id,
		const std::string& filename,
		const std::string& song_id);

std::string remove_file(
		const std::string& user_id,
		const std::string& filename);

std::string remove_file_from_library(
		const std::string& library_user_id,
		const std::string& song_username,
		const std::string& filename);

std::string share(
		const std::string& song_owner_id,
		const std::string& library_username,
		const std::string& filename);

std::string revoke_share(
		const std::string& song_owner_id,
		const std::string& library_username,
		const std::string& filename);
} //namespace query
} //namespace server
} //namespace musync
