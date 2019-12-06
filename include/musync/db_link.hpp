#ifndef MUSYNC_DB_LINK
#define MUSYNC_DB_LINK
#include <fstream>
#include <string>
#include <vector>
#include <memory>
namespace musync {
namespace server {
class db_link {
public:
	virtual ~db_link() {}

	// Gets a filename, which can uniquely identify the file to be sent
	// so that there are no clashes
	virtual std::string get_next_file_id(
			const std::string& user_id) = 0;

	virtual bool list_files(
			const std::string& user_id,
			std::vector< std::pair<std::string,std::string> >& files) = 0;

	virtual bool username_from_id(
			const std::string& user_id,
			std::string& username) = 0;
/*
	virtual bool filename_from_id(
			const std::string& user_id,
			const std::string& song_id,
			std::string& filename) = 0;
*/
	virtual bool does_file_exist(
			const std::string& user_id,
			const std::string& filename) = 0;

	virtual bool is_file_in_library(
			const std::string& user_id,
			const std::string& song_username,
			const std::string& filename) = 0;

	// When the stream has been written to, add file logs the new file
	virtual bool add_file(
			const std::string& user_id,
			const std::string& filename) = 0;

	virtual bool remove_file(
			const std::string& user_id,
			const std::string& filename) = 0;

	virtual bool remove_file_from_library(
			const std::string& user_id,
			const std::string& song_username,
			const std::string& filename) = 0;

	// User management

	// The following functions return true if they were successful,
	// and false if they failed because of a user error (mistyping the password)
	// and will throw an exception if it is a program error (can't connect to the database)
	virtual bool try_login(
			const std::string& user_or_email,
			const std::string& passwd,
			std::string& user_id) = 0;
	virtual bool try_register(
			const std::string& username,
			const std::string& email,
			const std::string& passwd,
			const std::string& first_name) = 0;
	virtual bool try_register(
			const std::string& username,
			const std::string& email,
			const std::string& passwd,
			const std::string& first_name,
			const std::string& last_name) = 0;

	// File sharing
	virtual bool share(
			const std::string& song_owner_id,
			const std::string& library_username,
			const std::string& filename) = 0;

	virtual bool revoke_share(
			const std::string& song_owner_id,
			const std::string& library_username,
			const std::string& filename) = 0;
};
typedef std::shared_ptr<db_link> db_link_ptr;
}
}
#endif
