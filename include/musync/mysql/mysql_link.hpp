#ifndef MUSYNC_MYSQL_LINK
#define MUSYNC_MYSQL_LINK
#include <musync/db_link.hpp>
#include <musync/config_vars.hpp>
#include <musync/mysql/mysql_error.hpp>
#include <musync/mysql/queries.hpp>
#include <mysql.h>
#include <memory>
#include <vector>
#include <utility>
#include <iostream>
namespace musync {
namespace server {
class mysql_link : public db_link {
public:
	mysql_link() = delete;
	mysql_link(config_ptr vars_global);
	~mysql_link();
	std::string get_next_file_id(
			const std::string& user_id);
	bool try_login(
			const std::string& username_or_email,
			const std::string& passwd,
			std::string& user_id);
	bool try_register(
			const std::string& username,
			const std::string& email,
			const std::string& passwd,
			const std::string& first_name);
	bool try_register(
			const std::string& username,
			const std::string& email,
			const std::string& passwd,
			const std::string& first_name,
			const std::string& last_name);
	bool list_files(
			const std::string& user_id,
			std::vector< std::pair<std::string,std::string> >& files);
	bool username_from_id(
			const std::string& user_id,
			std::string& username);
	bool is_file_in_library(
			const std::string& user_id,
			const std::string& song_username,
			const std::string& filename);
	bool does_file_exist(
			const std::string& user_id,
			const std::string& file);
	/*
	bool filename_from_id(
			const std::string& user_id,
			const std::string& song_id,
			std::string& filename);
	*/
	bool add_file(
			const std::string& user_id,
			const std::string& filename);
	bool remove_file(
			const std::string& user_id,
			const std::string& filename);
	bool remove_file_from_library(
			const std::string& user_id,
			const std::string& song_username,
			const std::string& filename);
	bool share(
			const std::string& song_owner_id,
			const std::string& library_username,
			const std::string& filename);

	bool revoke_share(
			const std::string& song_owner_id,
			const std::string& library_username,
			const std::string& filename);
private:
	config_ptr vars;
	MYSQL *conn;
};
}
}
#endif
